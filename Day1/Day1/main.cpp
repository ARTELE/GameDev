// Day1.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include <ctime>
#include <assert.h>
using namespace std;

class Game
{
	int answer = 0;
	int count = 0;
public:
	Game(){}

	void Initialize()
	{
		srand((int)time(0));
		answer = rand() % 1000;
	}

	void UpdateFrame()
	{
		int num = 0;
		while(true)
		{ 
			std::cout << "Please input num:";
			std::cin >> num;
			
			if (num < answer)
			{
				std::cout << "Smaller than answer" << std::endl;
			}
			else if (num > answer)
			{
				std::cout << "Bigger than answer" << std::endl;
			}
			else
			{
				std::cout << "You win! I will crash!" << std::endl;
				_sleep(1000);
				assert(0);
			}

			count++;
			if (count > 10)
			{
				std::cout << "You are as stupid as a goose! Bye." << std::endl;
				_sleep(1000);
				return;
			}
		}
	}
};

int main()
{
	Game game;
	game.Initialize();
	game.UpdateFrame();
	return 0;
}
