#include "Game.h"
#include "ResourceManager.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <GLFW/glfw3.h>

Game::Game(unsigned int width, unsigned int height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height),
	PLAYER_SIZE(100.0f, 20.0f), PLAYER_VELOCITY(500.0f),
	INITIAL_BALL_VELOCITY(100.0f, -350.0f), BALL_RADIUS(12.5f)
{
}

Game::~Game()
{
	delete Renderer;
	delete Player;
}

void Game::Init()
{
	// Load shaders
	ResourceManager::LoadShader("assets/shaders/sprite.vert", "assets/shaders/sprite.frag", nullptr, "sprite");

	// Configure shaders
	glm::mat4 projectionMatrix = glm::ortho(0.0f, 
		static_cast<float>(this->Width),
		static_cast<float>(this->Height),
		0.0f, -1.0f,1.0f);
	
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").Use().SetMatrix4("projection", projectionMatrix);

	// Set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

	// Load texture
	ResourceManager::LoadTexture("assets/textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("assets/textures/awesomeface.png", true, "face");
	ResourceManager::LoadTexture("assets/textures/block.png", false, "block");
	ResourceManager::LoadTexture("assets/textures/block_solid.png", false, "block_solid");
	ResourceManager::LoadTexture("assets/textures/paddle.png", true, "paddle");

	// Load levels
	GameLevel one;
	one.Load("assets/levels/one.lvl", this->Width, this->Height / 2);
	GameLevel two;
	one.Load("assets/levels/two.lvl", this->Width, this->Height / 2);
	GameLevel three;
	one.Load("assets/levels/three.lvl", this->Width, this->Height / 2);
	GameLevel four;
	one.Load("assets/levels/four.lvl", this->Width, this->Height / 2);

	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->currentLevel = 0;

	// Configure GameObjects
	glm::vec2 playerPos = glm::vec2(
		this->Width/ 2.0f - PLAYER_SIZE.x / 2.0f,
		this->Height - PLAYER_SIZE.y
	);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
											  -BALL_RADIUS * 2.0f);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::ProcessInput(float dt)
{
	if (this->State == GAME_ACTIVE)
	{
		float velocity = PLAYER_VELOCITY * dt;
		
		// Move player paddle
		if (this->Keys[GLFW_KEY_A] || this->Keys[GLFW_KEY_LEFT])
		{
			if (Player->Position.x >= 0.0f)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}
			
		}
		if (this->Keys[GLFW_KEY_D] || this->Keys[GLFW_KEY_RIGHT])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}

		// Start game
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
}

void Game::Update(float dt)
{
	Ball->Move(dt, this->Width);
}

void Game::Render()
{
	if (this->State == GAME_ACTIVE)
	{
		// Draw background
		Renderer->DrawSprite(ResourceManager::GetTexture("background"),
			glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f
		);

		// Draw level
		this->Levels[this->currentLevel].Draw(*Renderer);

		// Draw player
		this->Player->Draw(*Renderer);

		// Draw ball
		Ball->Draw(*Renderer);
	}
}
