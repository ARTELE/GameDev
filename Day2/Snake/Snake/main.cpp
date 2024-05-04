// Game.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include <vector>
#include <queue>
#include <windows.h>
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

enum PixelColor
{
	White = 0
};

enum PixelType
{
	None = 0,
	Square,
	Circle,
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
	std::vector<std::string> mPixelMap = { "  ","■", "●" };
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

		for (int i = 0; i < mBlockBuffer.size(); i++)
		{
			for (int j = 0; j < mBlockBuffer[i].size(); j++)
			{
				std::cout << mPixelMap[mBlockBuffer[i][j].mPixel];
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
		mRenderObject.mPixel = PixelType::Square;
	}

	void Update()
	{
		mRenderObject.mPositions.clear();
		//重新生成一个位置
		mRenderObject.mPositions.push_back(mPosition);
	}
};

class Snake : public GameObject
{
	std::queue<ivec2> mPositions;
	bool mNeedGrow = false;
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

	void Start()
	{
		mRenderObject.mPixel = PixelType::Circle;
		mPositions.push(ivec2(2, 2));
		mPositions.push(ivec2(2, 3));
		mPositions.push(ivec2(2, 4));
	}

	void Update()
	{
		// Update logic
		auto head = mPositions.back();
		head += ivec2::Right();
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
	}

	void UpdateFrame()
	{
		BlockDisplayDevice::Instance().AppenndRenderQueue(mWall.GetRenderObject());
		BlockDisplayDevice::Instance().AppenndRenderQueue(mSnake.GetRenderObject());
		BlockDisplayDevice::Instance().AppenndRenderQueue(mFood.GetRenderObject());
		BlockDisplayDevice::Instance().Display();

		// TODO:Get input

		if (mSnake.Collide(mFood.GetPosition()))
		{	
			mSnake.Grow();
		}

		mSnake.Update();
		mFood.Update();

		BlockDisplayDevice::Instance().Clear();
	}
};

int main()
{
	Game game;
	game.Initialize();
	while(true)
	{ 
		system("cls");
		game.UpdateFrame();
		_sleep(500);
	}
	return 0;
}
