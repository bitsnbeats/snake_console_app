#include <iostream>
#include <list>
#include<string>
#include<random>
#include<chrono>
#include<fstream>
#include <Windows.h>


using namespace std;

unsigned seed = chrono::system_clock::now().time_since_epoch().count();

int nScreenWidth = 120;
int nScreenHeight = 30;

default_random_engine generator(seed);
uniform_int_distribution<int> rand_x(1, nScreenWidth - 2);
uniform_int_distribution<int> rand_y(2, nScreenHeight - 2);

struct SnakeSegment
{
	int x;
	int y;
};

struct Food
{
	int x;
	int y;

	void moveFood()
	{
		x = rand_x(generator);
		y = rand_y(generator);
	}
};



int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	screen[nScreenWidth * nScreenHeight - 1] = '\0';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	bool start_screen = true;

	wifstream fin;
	wofstream fout;

	list<int> hi_score(10, 0);
	
	while (1)
	{
		//Initilize Hi Score
		fin.open("hi_score.txt");

		if (!fin)
		{

			fout.open("hi_score.txt");

			if (!fout) return 1;
			fout.close();

			fin.open("hi_score.txt");
			if (!fin) return 1;
		}

		for (auto it = hi_score.begin(); it != hi_score.end(); it++)
			fin >> *it;




		//Start Screen Loop
		while (start_screen)
		{
			//Clear Screen
			for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
			wsprintf(&screen[2 * nScreenWidth + 40], L"WELCOME TO SNAKE! PRESS ENTER TO PLAY");

			wsprintf(&screen[4 * nScreenWidth + 53], L"HIGH SCORE:");

			for (auto it = hi_score.begin(); it != hi_score.end(); it++)
			{
				int i = distance(hi_score.begin(), it);
				wstring hi_score_text = to_wstring(i + 1) + L"  : ";
				if (i == 9) hi_score_text = L"10 : ";

				if (*it) hi_score_text += to_wstring(*it);
				else hi_score_text += L"__";

				wsprintf(&screen[(i + 5) * nScreenWidth + 53], hi_score_text.c_str());
			}



			if (GetAsyncKeyState(VK_RETURN))
				start_screen = false;

			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
		}

		list<SnakeSegment> SnakeBody;
		for (int i = 0; i < 6; i++)
		{
			SnakeSegment segment{ i, 15 };
			SnakeBody.push_front(segment);
		}

		
		Food food = { 20, 10 };

		//Variables for game
		int header_UI = 1;
		int score = 0;
		wstring score_text;

		wchar_t segment_char = L'O';
		wchar_t head_char = L'@';
		wchar_t food_char = L'$';

		bool isDead = false;

		bool pressUp = false;
		bool pressDown = false;
		bool pressLeft = false;
		bool pressRight = false;

		bool pressUpOld = false;
		bool pressDownOld = false;
		bool pressLeftOld = false;
		bool pressRightOld = false;

		int x_velocity = 1;
		int y_velocity = 0;
		int growth_rate = 10;
		int growth = 0;

		auto interval = 150ms;
		auto speed_increase = 0ms;
		auto speed_increase_rate = 5ms;

		while (!isDead)
		{
			//Clear Screen
			for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

			//Timing and Input

			auto time1 = chrono::system_clock::now();

			if (y_velocity)
				interval = 220ms - speed_increase;
			else
				interval = 150ms - speed_increase;

			while (chrono::system_clock::now() - time1 < interval)
			{
				pressUp = GetAsyncKeyState(VK_UP);
				pressDown = GetAsyncKeyState(VK_DOWN);
				pressLeft = GetAsyncKeyState(VK_LEFT);
				pressRight = GetAsyncKeyState(VK_RIGHT);

				if (pressUp && !pressUpOld && y_velocity == 0)
				{
					x_velocity = 0;
					y_velocity = -1;
				}
				if (pressDown && !pressDownOld && y_velocity == 0)
				{
					x_velocity = 0;
					y_velocity = 1;
				}
				if (pressLeft && !pressLeftOld && x_velocity == 0)
				{
					x_velocity = -1;
					y_velocity = 0;
				}
				if (pressRight && !pressRightOld && x_velocity == 0)
				{
					x_velocity = 1;
					y_velocity = 0;
				}

				pressUpOld = pressUp;
				pressDownOld = pressDown;
				pressLeftOld = pressLeft;
				pressRightOld = pressRight;
			}




			//Game Logic
				//Move Snake
			SnakeBody.push_front({ SnakeBody.front().x + x_velocity, SnakeBody.front().y + y_velocity });

			if (growth)
				growth--;
			else
				SnakeBody.pop_back();

				//Check for Food
			if (SnakeBody.front().x == food.x && SnakeBody.front().y == food.y)
			{
				growth += growth_rate;

				speed_increase += speed_increase_rate;
				if (speed_increase > 145ms)
					speed_increase = 145ms;

				score += 1;

				MOVE_FOOD:
				food.moveFood();

				for (auto segment : SnakeBody)
				{
					if (segment.x == food.x && segment.y == food.y)
						goto MOVE_FOOD;
				}
			}
				


				//Check for death
			if (SnakeBody.front().x == nScreenWidth || SnakeBody.front().x == 0 || SnakeBody.front().y == nScreenHeight - 1 || SnakeBody.front().y == header_UI)
				isDead = true;
			
			for (auto it = (++SnakeBody.begin()); it != SnakeBody.end(); it++)
			{
				if ((*it).x == SnakeBody.front().x && (*it).y == SnakeBody.front().y)
					isDead = true;
			}

			if (isDead)
			{
				x_velocity = 0;
				y_velocity = 0;
				segment_char = L'+';
				head_char = L'#';

				
			}

				//Update Score
			score_text = L"Score: " + to_wstring(score);



			//Display stuff
		
			    //Display UI
			for (int i = 0; i < nScreenWidth; i++)
			{
				screen[header_UI * nScreenWidth + i] = L'=';
			}

			for (int i = 0; i < nScreenWidth; i++)
			{
				screen[(nScreenHeight - 1) * nScreenWidth + i] = L'=';
			}

			for (int i = 0; i < nScreenHeight; i++)
			{
				screen[i * nScreenWidth] = L'|';
				screen[i * nScreenWidth + nScreenWidth - 1] = L'|';
			}

			for (int i = 0; i < score_text.length(); i++) screen[55 + i] = score_text[i];

			    //Display Snake
			for (auto segment : SnakeBody)
			{
				screen[segment.y * nScreenWidth + segment.x] = segment_char;
			}

			screen[SnakeBody.front().y * nScreenWidth + SnakeBody.front().x] = head_char;

				//Display Food
			screen[food.y * nScreenWidth + food.x] = food_char;

			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
		}

		//Check for High Score
		fin.close();
		fout.open("hi_score.txt");

		for (auto it = hi_score.begin(); it != hi_score.end(); ++it)
		{
			int i = distance(hi_score.begin(), it);

			if (score > *it && score != 0)
			{
				wstring hi_score_message = L"YOU GOT THE NO. " + to_wstring(i + 1);
				hi_score_message += L" HIGH SCORE!";

				wsprintf(&screen[14 * nScreenWidth + 40], hi_score_message.c_str());

				hi_score.insert(it, score);
				hi_score.pop_back();

				break;
			}
		}

		for (auto it = hi_score.begin(); it != hi_score.end(); it++)
			fout << *it << endl;

		wsprintf(&screen[15 * nScreenWidth + 40], L"YOU DIED! PRESS ENTER TO PLAY AGAIN");
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth* nScreenHeight, { 0, 0 }, & dwBytesWritten);

		fout.close();

		while (isDead)
		{
			if (GetAsyncKeyState(VK_RETURN))
				isDead = false;
		}
	}

	fin.close();
	fout.close();

	return 0;
}

