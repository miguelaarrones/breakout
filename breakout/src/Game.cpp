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
	ResourceManager::LoadTexture("assets/textures/awesomeface.png", true, "face");
}

void Game::ProcessInput(float dt)
{
}

void Game::Update(float dt)
{
}

void Game::Render()
{
	Renderer->DrawSprite(ResourceManager::GetTexture("face"),
		glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f,
		glm::vec3(0.0f, 1.0f, 0.0f));
}
