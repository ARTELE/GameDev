// Game.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include <vector>
#include <queue>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <cassert>
using namespace std;

struct ivec2
{
	int mX = 0, mY = 0;
	ivec2() {}
	ivec2(int x, int y) : mX(x), mY(y) { }

	ivec2& operator += (const ivec2& vec)
	{
		mX += vec.mX;
		mY += vec.mY;

		return *this;
	}

	bool operator == (const ivec2& vec)
	{
		return mX == vec.mX && mY == vec.mY;
	}

	static ivec2 Left() { return ivec2(-1, 0); }
	static ivec2 Right() { return ivec2(1, 0); }
	static ivec2 Up() { return ivec2(0, -1); }
	static ivec2 Down() { return ivec2(0, 1); }
};

enum InputKey
{
	Left = 75,
	Right = 77,
	Up = 72,
	Down = 80
};

class InputManager
{
	int mCurrentKey = 0;
public:
	void CollectInputKey()
	{
		if (_kbhit())
		{
			mCurrentKey = _getch();
			if (mCurrentKey == 224)
				mCurrentKey = _getch();
		}
	}

	int GetCurrentKey() const { return mCurrentKey; }

	static InputManager& Instance()
	{
		static InputManager inputManager;
		return inputManager;
	}
};

enum PixelColor
{
	White = 0,
	Red = 4,
	Yellow = 14
};

enum PixelType
{
	None = 0,
	Square,
	Circle,
	Star
};

struct ScreenAttribute
{
	PixelType mPixel = PixelType::None;
	PixelColor mColor = PixelColor::White;
};

struct RenderElement
{
	ivec2 mPosition;
	ScreenAttribute mAttribute;
};

struct RenderObject
{
	PixelType mPixel = PixelType::None;
	PixelColor mColor = PixelColor::White;
	std::vector<ivec2> mPositions;
};

class BlockDisplayDevice
{
	std::vector<std::vector<ScreenAttribute>> mBlockBuffer;
	std::vector<std::string> mPixelMap = { "  ","■", "●", "★"};
	std::vector<RenderElement> mRenderQueue;
public:

	BlockDisplayDevice(int w, int h)
	{
		mBlockBuffer.resize(h);
		for (auto& buffer : mBlockBuffer)
		{
			buffer.resize(w);
		}
	}

	int Width() const { return mBlockBuffer.empty() ? 0 : mBlockBuffer[0].size(); }
	int Height() const { return mBlockBuffer.size(); }

	void Clear()
	{
		for (int i = 0; i < mBlockBuffer.size(); i++)
		{
			for (int j = 0; j < mBlockBuffer[i].size(); j++)
			{
				mBlockBuffer[i][j] = ScreenAttribute();
			}
		}
	}

	void Display()
	{
		for (auto& block : mRenderQueue)
		{
			int x = block.mPosition.mX;
			int y = block.mPosition.mY;
			if (x < mBlockBuffer.size() && y < mBlockBuffer[x].size())
			{
				mBlockBuffer[block.mPosition.mX][block.mPosition.mY] = block.mAttribute;
			}
		}

		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		
		for (int i = 0; i < mBlockBuffer.size(); i++)
		{
			for (int j = 0; j < mBlockBuffer[i].size(); j++)
			{
				int color = mBlockBuffer[j][i].mColor;
				if (color == PixelColor::White)
				{
					std::cout << mPixelMap[mBlockBuffer[j][i].mPixel];
				}
				else
				{ 
					SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
					std::cout << mPixelMap[mBlockBuffer[j][i].mPixel];
					SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
				}
			}

			std::cout << std::endl;
		}

		mRenderQueue.clear();
	}

	void AppenndRenderQueue(RenderObject& renderData)
	{
		for (int i = 0; i < renderData.mPositions.size(); i++)
		{
			mRenderQueue.push_back(RenderElement{renderData.mPositions[i], ScreenAttribute{renderData.mPixel, renderData.mColor}});
		}
	}

	static BlockDisplayDevice& Instance()
	{
		static BlockDisplayDevice device(30, 30);
		return device;
	}
};

class GameObject
{
protected:
	RenderObject mRenderObject;
public:
	GameObject() {}
	RenderObject& GetRenderObject()
	{
		return mRenderObject;
	}

	virtual void Update() {}
};

class Food : public GameObject
{
	ivec2 mPosition;
public:

	void SetPosition(const ivec2 pos)
	{
		mPosition = pos;
	}

	ivec2 GetPosition()
	{
		return mPosition;
	}

	void Start()
	{
		mRenderObject.mPixel = PixelType::Star;
		mRenderObject.mColor = PixelColor::Red;
	}

	void Update()
	{
		mRenderObject.mPositions.clear();
		mRenderObject.mPositions.push_back(mPosition);
	}
};

class Snake : public GameObject
{
	std::queue<ivec2> mPositions;
	bool mNeedGrow = false;
	ivec2 mDirection = ivec2::Right();
public:
	
	void Grow()
	{
		mNeedGrow = true;
	}

	bool Collide(const ivec2& otherPos)
	{
		bool result = false;
		auto pos = mPositions.back();
		if (otherPos == pos)
			result = true;
		return result;
	}
	bool Collide(const std::vector<ivec2>& otherPos)
	{
		for (int i = 0; i < otherPos.size(); i++)
		{
			auto head = mPositions.back();
			if (otherPos[i] == head)
				return true;
		}

		return false;
	}

	void SetDirection(const ivec2& dir)
	{
		mDirection = dir;
	}

	void Start()
	{
		mRenderObject.mPixel = PixelType::Circle;
		mRenderObject.mColor = PixelColor::Yellow;
		mPositions.push(ivec2(2, 2));
		mPositions.push(ivec2(2, 3));
		mPositions.push(ivec2(2, 4));
	}

	void Update()
	{
		// Update logic
		auto head = mPositions.back();
		head += mDirection;
		if (!mNeedGrow)
			mPositions.pop();
		mPositions.push(head);
		mNeedGrow = false;

		// Update render data
		mRenderObject.mPositions.clear();
		int queueSize = mPositions.size();
		for (int i = 0; i < queueSize; i++)
		{
			auto pos = mPositions.front();
			mRenderObject.mPositions.push_back(pos);
			mPositions.push(pos);
			mPositions.pop();
		}
	}
};

class Game
{
	GameObject mWall;
	Food mFood;
	Snake mSnake;

	time_t mCurrentTime = 0, mLastTime = 0;

public:
	Game() {}

	void Initialize()
	{
		auto& wall = mWall.GetRenderObject();
		wall.mPixel = PixelType::Square;
		for (int i = 0; i < 30; i++)
		{
			wall.mPositions.push_back(ivec2(i, 0));
			wall.mPositions.push_back(ivec2(0, i));
			wall.mPositions.push_back(ivec2(i, 29));
			wall.mPositions.push_back(ivec2(29, i));
		}

		mSnake.Start();
		mFood.Start();
		mFood.SetPosition(ivec2(10, 4));

		mCurrentTime = clock();
		mLastTime = mCurrentTime;
	}

	void UpdateFrame()
	{
		mCurrentTime = clock();
		// Get input
		InputManager::Instance().CollectInputKey();
		{
			switch (InputManager::Instance().GetCurrentKey())
			{
				case InputKey::Left:
					mSnake.SetDirection(ivec2::Left());
					break;
				case InputKey::Right:
					mSnake.SetDirection(ivec2::Right());
					break;
				case InputKey::Up:
					mSnake.SetDirection(ivec2::Up());
					break;
				case InputKey::Down:
					mSnake.SetDirection(ivec2::Down());
					break;
			}
		}

		if (mCurrentTime - mLastTime > 100)
		{
			system("cls");

			// Render update
			BlockDisplayDevice::Instance().AppenndRenderQueue(mWall.GetRenderObject());
			BlockDisplayDevice::Instance().AppenndRenderQueue(mSnake.GetRenderObject());
			BlockDisplayDevice::Instance().AppenndRenderQueue(mFood.GetRenderObject());
			BlockDisplayDevice::Instance().Display();

			// Game logic update
			if (mSnake.Collide(mFood.GetPosition()))
			{	
				mSnake.Grow();

				int x = (rand() % 28) + 1;
				int y = (rand() % 28) + 1;
				mFood.SetPosition(ivec2(x, y));
			}

			if (mSnake.Collide(mWall.GetRenderObject().mPositions))
			{
				assert(0);
			}

			mSnake.Update();
			mFood.Update();

			BlockDisplayDevice::Instance().Clear();
			mLastTime = mCurrentTime;
		}
	}
};

int main()
{
	srand((int)time(0));
	Game game;
	game.Initialize();

	while(true)
	{
		game.UpdateFrame();
		_sleep(1);
	}
	return 0;
}
