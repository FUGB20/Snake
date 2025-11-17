#include <iostream>
#include <raylib.h>
#include <random>
#include <deque>
#include <raymath.h>

constexpr int WINDOW_WIDTH{ 900 };
constexpr int WINDOW_HEIGHT{ 900 };
constexpr int cellSize{ 30 };
constexpr int cellCount{ 25 };
constexpr int offset{ 75 };
constexpr Color green{ 173, 204, 96, 255 };
constexpr Color darkGreen{ 43, 51, 24, 255 };

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(0, cellCount - 1);

class Snake
{
private:
	std::deque<Vector2> snake_body{ Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
	Vector2 snake_direction{ 1,0 };
	double snake_interval{ 0.5 };

public:
	void update()
	{
		snakeDirection();
		if (snakeSpeed())
		{
			snake_body.pop_back();
			snake_body.push_front(snake_body[0] + snake_direction);
			snakeStop();
		}
	}

	void growSnake()
	{
		snake_body.push_front(snake_body[0] + snake_direction);
	}

	bool snakeSpeed() const
	{
		static double lastTimeInterval{};
		double currentTime{ GetTime() };

		if (currentTime - lastTimeInterval >= snake_interval)
		{
			lastTimeInterval = currentTime;
			return true;
		}
		else
			return false;
	}

	void snakeDirection()
	{
		if (IsKeyPressed(KEY_LEFT) && snake_direction.x != 1)
		{
			snake_direction = { -1,0 };
		}

		if (IsKeyPressed(KEY_RIGHT) && snake_direction.x != -1)
		{
			snake_direction = { 1,0 };
		}

		if (IsKeyPressed(KEY_DOWN) && snake_direction.y != -1)
		{
			snake_direction = { 0,1 };
		}

		if (IsKeyPressed(KEY_UP) && snake_direction.y != 1)
		{
			snake_direction = { 0,-1 };
		}
	}

	bool snakeStop()
	{
		if (snake_body[0].x == -1 || snake_body[0].x == cellCount)
		{
			return true;
		}

		if (snake_body[0].y == -1 || snake_body[0].y == cellCount)
		{
			return true;
		}

		if (checkBodyCollision())
		{
			return true;
		}
		return false;
	}

	bool checkBodyCollision()
	{
		for (std::size_t i = 1; i < snake_body.size(); i++)
		{
			if (snake_body[0] == snake_body[i])
				return true;
		}
		return false;
	}

	void snakeReset()
	{
		snake_body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
		snake_direction = { 1,0 };
	}

	void draw()
	{
		float x{};
		float y{};
		Rectangle cell{};

		for (std::size_t i = 0; i < snake_body.size(); i++)
		{
			x = snake_body[i].x;
			y = snake_body[i].y;
			cell = { offset + x * cellSize,offset + y * cellSize,
				static_cast<float>(cellSize),static_cast<float>(cellSize) };
			if (i == 0)
			{
				DrawRectangleRounded(cell, 0.5, 6, YELLOW);
				continue;
			}
			DrawRectangleRounded(cell, 0.5, 6, darkGreen);
		}
	}

	Vector2 getPos() const { return snake_body[0]; }

	std::deque<Vector2> getSnake() const { return snake_body; }
};

class Food
{
private:
	Texture2D food_texture{};
	Vector2 food_position{};

public:
	Food()
	{
		food_texture = LoadTexture("assets/food.png");
		randomizePosition();
	}
	~Food()
	{
		UnloadTexture(food_texture);
	}

	void randomizePosition()
	{
		food_position.x = dist(gen);
		food_position.y = dist(gen);
	}

	bool checkFoodOverlap(std::deque<Vector2> snake) const
	{
		for (std::size_t i = 0; i < snake.size(); i++)
		{
			if (food_position == snake[i])
			{
				return true;
			}
		}
		return false;
	}

	void resolveFoodOverlap(std::deque<Vector2> snake)
	{
		while (checkFoodOverlap(snake))
		{
			randomizePosition();
		}
	}

	void draw() const
	{
		DrawTexture(food_texture, offset + food_position.x * cellSize, offset + food_position.y * cellSize, WHITE);
	}

	Vector2 getPos() const { return food_position; }
};

class Game
{
private:
	Snake snake{};
	Food food{};
	int score{};
	Sound eat{};
	Sound wall{};

public:
	Game()
	{
		InitAudioDevice();
		eat = LoadSound("assets/eat.mp3");
		wall = LoadSound("assets/wall.mp3");
	}
	~Game()
	{
		UnloadSound(eat);
		UnloadSound(wall);
		CloseAudioDevice();
	}

	void update()
	{
		snake.update();
		checkFoodCollision();
		food.resolveFoodOverlap(snake.getSnake());
		if (snake.snakeStop())
		{
			PlaySound(wall);
			gameover(true);
		}
	}

	void checkFoodCollision()
	{
		if (food.getPos() == snake.getPos())
		{
			score++;
			PlaySound(eat);
			snake.growSnake();
			food.randomizePosition();
		}
	}

	void draw()
	{
		snake.draw();
		food.draw();
		DrawRectangleLinesEx(Rectangle{ static_cast<float>(offset - 5), static_cast<float>(offset - 5) ,
			static_cast<float>(cellCount * cellSize + 10), static_cast<float>(cellCount * cellSize + 10) }, 5, darkGreen);
		DrawText("Snake", offset - 5, 20, 40, darkGreen);
		DrawText(TextFormat("%i", score), static_cast<float>(offset - 5),
			static_cast<float>(offset + cellCount * cellSize + 10), 40, darkGreen);
	}

	void gameover(bool stop)
	{
		std::cout << "game over\n";
		system("PAUSE");
		score = 0;
		snake.snakeReset();
	}
};

int main()
{
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snake");
	Game game{};

	while (!WindowShouldClose())
	{
		game.update();

		BeginDrawing();
		ClearBackground(green);

		game.draw();

		EndDrawing();
	}

	CloseWindow();
	return 0;
}