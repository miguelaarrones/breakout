#include "Game.h"
#include "ResourceManager.h"
#include <glm/ext/matrix_clip_space.hpp>

Game::Game(unsigned int width, unsigned int height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
	delete Renderer;
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
}

void Game::ProcessInput(float dt)
{
}

void Game::Update(float dt)
{
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
	}
}
