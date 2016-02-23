#include <SDL.h>
#include <iostream>
#include <SDL_image.h>
#include <string>
#include <cmath>
#include <Windows.h>

//Globals
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//These variables were made so that the classes could interact.
int yPaddle;
int yEnemy;
int yBall;
int paddleVelocity;
int enemyVelocity;
int ballxVelocity;
int ballyVelocity;
int xBall;
int paddleScore = 0;
int enemyScore = 0;

//SDL Globals
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* gTexture = NULL;

//Initializes the window, renderer, and PNG/SDL_Image stuff
bool init()
{
	bool success = true;

	//Initialize SDL. < 0 means that initialization failed.
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Couldnt initialize SDL" << std::endl;
		success = false;
	}
	else
	{
		//Creating the window.
		window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, NULL);
		if (window == NULL)
		{
			std::cout << "Couldnt create window" << std::endl;
			success = false;
		}
		else
		{
			//Creating the renderer. The Flags make the renderer match the monitor refresh rate.
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL)
			{
				std::cout << "Couldnt create renderer" << std::endl;
				success = false;
			}
			else
			{
				//Setting the renderer color. Still not sure why this has to be done.
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Making it possible to load PNG images.
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					std::cout << "Couldnt initialize SDL_Image" << std::endl;
					success = false;
				}
			}
		}
	}

	return success;
}

//Function for giving textures their image.
SDL_Texture* loadTexture(std::string path)
{
	//Loads a texture through this process. Texture made. Surface made. Texture made into surface. Surface deleted. Texture returned.
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
	SDL_FreeSurface(loadedSurface);
	
	return newTexture;
}

//Loads all of the pictures needed.
void loadMedia()
{
	//Backgroung picture.
	gTexture = loadTexture("Sprites/LoadingScreen.png");
}

//Shuts down everything before exiting the application.
void close()
{
	//Exits out of everything. Ball and Paddle sprites not included.
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;
	SDL_DestroyRenderer(renderer);
	window = NULL;
	renderer = NULL;
	IMG_Quit();
	SDL_Quit();
}

class Paddle
{
public:
	//Initializes the paddle with all of its characteristics.
	Paddle()
	{
		paddleTexture = loadTexture("Sprites/Paddle.png");
		y = 470 - 96;
		maxJumpVel = -15;
		maxFallVel = 15;
		jumpBoost = -15;
	}

	//Moves the paddle based on the velocity. Then changes the velocity. Also puts limits on minimum and maximum values. Updates global variables.
	void Move()
	{
		y = y + velocity;
		velocity = velocity + 1;

		if (y < 10)
		{
			y = 10;
			velocity = 0;
		}
		if (y > 470 - 96)
		{
			y = 470 - 96;
			velocity = 0;
		}
		if (velocity > maxFallVel)
		{
			velocity = maxFallVel;
		}
		if (velocity < maxJumpVel)
		{
			velocity = maxJumpVel;
		}
		yPaddle = y;
		paddleVelocity = velocity;
	}

	//Renders the paddle to the screen.
	void render(SDL_Renderer* renderer)
	{
		SDL_Rect renderQuad = { 10, y, 32, 96 };
		SDL_RenderCopy(renderer, paddleTexture, NULL, &renderQuad);
	}

	//Takes input to jump.
	void handleEvent(SDL_Event& e)
	{
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
		{
			velocity = jumpBoost;
		}
	}
private:
	int y;
	int velocity;
	int maxJumpVel;
	int maxFallVel;
	int jumpBoost;
	SDL_Texture* paddleTexture;
};

//Pretty much the same as 'Paddle'. Just small differences in AI.
class EnemyPaddle
{
public:
	EnemyPaddle()
	{
		paddleTexture = loadTexture("Sprites/EnemyPaddle.png");
		y = 470 - 96;
		maxJumpVel = -15;
		maxFallVel = 15;
		jumpBoost = -15;
		velocity = 0;
	}

	void Move()
	{
		y = y + velocity;
		velocity = velocity + 1;

		if (y < 10)
		{
			y = 10;
			velocity = 0;
		}
		if (y > 470 - 96)
		{
			y = 470 - 96;
			velocity = 0;
		}
		if (velocity > maxFallVel)
		{
			velocity = maxFallVel;
		}
		if (velocity < maxJumpVel)
		{
			velocity = maxJumpVel;
		}

		if (y - 48 > yBall && ballxVelocity > 0 && velocity > -13)
		{
			jump();
		}
		yEnemy = y;
		enemyVelocity = velocity;
	}

	void jump()
	{
		velocity = jumpBoost;
	}

	void render(SDL_Renderer* renderer)
	{
		SDL_Rect renderQuad = { SCREEN_WIDTH - 42, y, 32, 96 };
		SDL_RenderCopy(renderer, paddleTexture, NULL, &renderQuad);
	}

private:
	int y;
	SDL_Texture* paddleTexture;
	int maxJumpVel;
	int maxFallVel;
	int jumpBoost;
	int velocity;
};

class Ball
{
public:
	Ball()
	{
		ballTexture = loadTexture("Sprites/Ball.png");
		xPos = 480;
		yPos = 224;
		xVel = 10;
		yVel = 5;
	}

	//Moves ball and a lot of collision detection. Also some stuff about curving the ball when its hit at an angle.
	void Move()
	{
		xPos = xPos + xVel;
		yPos = yPos + yVel;

		if (xPos > SCREEN_WIDTH)
		{
			xPos = 160;
			yPos = 224;
			yVel = rand() % 19 - 10;
			paddleScore += 1;
			Sleep(200);
		}
		if (xPos < 0)
		{
			xPos = 480;
			yPos = 224;
			yVel = rand()%19 - 10;
			enemyScore += 1;
			Sleep(200);
		}
		if (yPos > SCREEN_HEIGHT - 32)
		{
			yPos = SCREEN_HEIGHT- 32;
			yVel = -yVel;
		}
		if (yPos < 0)
		{
			yPos = 0;
			yVel = -yVel;
		}
		if (xPos / 42 == 1 && xPos % 42 <= std::abs(xVel) && yPos > yPaddle - 32 && yPos < yPaddle + 96 && xVel < 0)
		{
			xVel = -xVel;
			yVel = yVel + paddleVelocity / 2;
		}
		if (xPos / (SCREEN_WIDTH - 74) == 1 && xPos % (SCREEN_WIDTH - 74) <= xVel && yPos > yEnemy - 32 && yPos < yEnemy + 96 && xVel > 0)
		{
			yVel = yVel + enemyVelocity / 2;
			xVel = -xVel;
		}
		if (abs(yVel) > 12)
		{
			yVel = yVel / abs(yVel) * 12;
		}
		yBall = yPos;
		xBall = xPos;
		ballxVelocity = xVel;
		ballyVelocity = yVel;
	}

	//Renders the ball to the screen.
	void Render(SDL_Renderer* renderer)
	{
		SDL_Rect renderQuad = { xPos, yPos, 32, 32 };
		SDL_RenderCopy(renderer, ballTexture, NULL, &renderQuad);
	}
private:
	int xPos, yPos, xVel, yVel, xBall;
	SDL_Texture* ballTexture;


};

class Scoreboard
{
public:
	Scoreboard()
	{
		scoreTexture = loadTexture("Sprites/Scoreboard.png");
	}

	//Renders the scores.
	void render(SDL_Renderer* renderer)
	{
		int pyCord = 128 * paddleScore;
		int eyCord = 128 * enemyScore;
		SDL_Rect RpaddleScore = { 0, pyCord, 128, 128 };
		SDL_Rect paddleRQ = { 0, 0, 128, 128 };
		SDL_Rect RenemyScore = { 0, eyCord, 128, 128 };
		SDL_Rect EnemyRQ = { 320, 0, 128, 128 };
		
		SDL_RenderCopy(renderer, scoreTexture, &RpaddleScore, &paddleRQ);
		SDL_RenderCopy(renderer, scoreTexture, &RenemyScore, &EnemyRQ);

	}
private:
	SDL_Texture* scoreTexture;
};

int main(int argc, char* args[]) 
{
	//All the processes needed to begin.
	init();
	SDL_Event e;
	loadMedia();
	Paddle paddle;
	Ball ball;
	EnemyPaddle enemyPaddle;
	Scoreboard scoreboard;

	//Some variables for flow control.
	bool gameStarted = false;
	bool quit = false;
	bool gameOver = false;

	//Main game loop.
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				SDL_Quit();
				quit = true;
			}
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_SPACE)
				{
					if (gameStarted == false && gameOver == false)
					{
						gTexture = loadTexture("Sprites/GameBackground.png");
						gameStarted = true;
					}
				}
			}
			paddle.handleEvent(e);
		}
		
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, gTexture, NULL, NULL);
		
		if (gameStarted == true)
		{
			scoreboard.render(renderer);
			paddle.Move();
			ball.Move();
			enemyPaddle.Move();
			paddle.render(renderer);
			ball.Render(renderer);
			enemyPaddle.render(renderer);
		}

		if (paddleScore > 10)
		{
			gameOver = true;
			gameStarted = false;
			SDL_RenderClear(renderer);
			gTexture = loadTexture("Sprites/WinScreen.png");
			SDL_RenderCopy(renderer, gTexture, NULL, NULL);
		}

		if (enemyScore > 10)
		{
			gameOver = true;
			gameStarted = false;
			SDL_RenderClear(renderer);
			gTexture = loadTexture("Sprites/LoseScreen.png");
			SDL_RenderCopy(renderer, gTexture, NULL, NULL);
		}
		
		SDL_RenderPresent(renderer);
	}
	
	
	close();
	return 0;
}