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
	delete Ball;
	delete Particles;
	delete Effects;
}

void Game::Init()
{
	// Load shaders
	ResourceManager::LoadShader("assets/shaders/sprite.vert", "assets/shaders/sprite.frag", nullptr, "sprite");
	ResourceManager::LoadShader("assets/shaders/particle.vert", "assets/shaders/particle.frag", nullptr, "particle");
	ResourceManager::LoadShader("assets/shaders/post_processing.vert", "assets/shaders/post_processing.frag", nullptr, "postprocessing");

	// Configure shaders
	glm::mat4 projectionMatrix = glm::ortho(0.0f, 
		static_cast<float>(this->Width),
		static_cast<float>(this->Height),
		0.0f, -1.0f,1.0f);
	
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projectionMatrix);

	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projectionMatrix);

	// Load texture
	ResourceManager::LoadTexture("assets/textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("assets/textures/awesomeface.png", true, "face");
	ResourceManager::LoadTexture("assets/textures/block.png", false, "block");
	ResourceManager::LoadTexture("assets/textures/block_solid.png", false, "block_solid");
	ResourceManager::LoadTexture("assets/textures/paddle.png", true, "paddle");
	ResourceManager::LoadTexture("assets/textures/particle.png", true, "particle");

	// Set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	Particles = new ParticleGenerator(
		ResourceManager::GetShader("particle"),
		ResourceManager::GetTexture("particle"),
		500
	);

	Effects = new PostProcessor(
		ResourceManager::GetShader("postprocessing"), 
		this->Width, 
		this->Height
	);

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

	// Check for collisions
	this->DoCollisions();

	// Check if ball passed the bottom edge
	if (Ball->Position.y >= this->Height)
	{
		this->ResetLevel();
		this->ResetPlayer();
	}

	// Update particles
	Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));

	if (EffectsShakeTime > 0.0f)
	{
		EffectsShakeTime -= dt;
		if (EffectsShakeTime <= 0.0f)
			Effects->Shake = false;
	}
}

void Game::Render()
{
	if (this->State == GAME_ACTIVE)
	{
		Effects->BeginRender();

		// Draw background
		Renderer->DrawSprite(ResourceManager::GetTexture("background"),
			glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f
		);

		// Draw level
		this->Levels[this->currentLevel].Draw(*Renderer);

		// Draw player
		this->Player->Draw(*Renderer);

		// Render particles
		Particles->Draw();

		// Draw ball
		Ball->Draw(*Renderer);

		Effects->EndRender();
		Effects->Render(glfwGetTime());

	}
}

void Game::DoCollisions()
{
	// Ball - Brick collision
	for (GameObject &box : this->Levels[this->currentLevel].Bricks)
	{
		if (!box.Destroyed)
		{
			Collision collision = CheckCollision(*Ball, box);
			
			if (std::get<0>(collision)) // If collision is true
			{
				// Destroy block if not solid
				if (!box.IsSolid)
					box.Destroyed = true;
				else
				{
					// If block is solid, enable shake effect
					EffectsShakeTime = 0.05f;
					Effects->Shake = true;
				}
				
				// Collision resolution
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);

				if (dir == LEFT || dir == RIGHT) // Horizontal collision
				{
					// Reverse horizontal velocity
					Ball->Velocity.x = -Ball->Velocity.x;

					// Relocate
					float penetration = Ball->Radius - std::abs(diff_vector.x);
					if (dir == LEFT)
						Ball->Position.x += penetration; // Move ball to right
					else 
						Ball->Position.x -= penetration; // Move ball to left
				}
				else // Vertical collision
				{
					// Reverse vertical velocity
					Ball->Velocity.y = -Ball->Velocity.y;

					// Relocate
					float penetration = Ball->Radius - std::abs(diff_vector.y);
					if (dir == UP)
						Ball->Position.x += penetration; // Move ball up
					else
						Ball->Position.x -= penetration; // Move ball down
				}
			}
		}
	}

	// Ball - Player collision
	Collision result = CheckCollision(*Ball, *Player);

	if (!Ball->Stuck && std::get<0>(result))
	{
		// Check where it hit in the board and change velocity abse on where it hit
		float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
		float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		float percentage = distance / (Player->Size.x / 2.0f);

		// Move acordingly
		float strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		//Ball->Velocity.y = -Ball->Velocity.y;
		Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
	}
}

void Game::ResetLevel()
{
	if (this->currentLevel == 0)
		this->Levels[0].Load("assets/levels/one.lvl", this->Width, this->Height / 2);
	else if (this->currentLevel == 1)
		this->Levels[1].Load("assets/levels/two.lvl", this->Width, this->Height / 2);
	else if (this->currentLevel == 2)
		this->Levels[2].Load("assets/levels/three.lvl", this->Width, this->Height / 2);
	else if (this->currentLevel == 3)
		this->Levels[3].Load("assets/levels/four.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

bool Game::CheckCollision(GameObject &one, GameObject &two)
{
	// Collision x-axis?
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;

	// Collision y-axis?
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;

	// Collision only if in both axes
	return collisionX && collisionY;
}

Collision Game::CheckCollision(BallObject &one, GameObject &two)
{
	// Get center point circle first
	glm::vec2 center(one.Position + one.Radius);

	// Calculate AABB info (center, half-extents)
	glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
	glm::vec2 aabb_center(
		two.Position.x + aabb_half_extents.x,
		two.Position.y + aabb_half_extents.y
	);

	// Get difference vector between both centers
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

	// Add clamped value to AABB_Center and we get the value of box closest to circle
	glm::vec2 closest = aabb_center + clamped;

	// Retrieve vector between center circle and closest point AABB and check if length <= radius
	difference = closest - center;

	if (glm::length(difference) <= one.Radius)
		return std::make_tuple(true, VectorDirection(difference), difference);
	else
		return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

Direction Game::VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, -1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};

	float max = 0.0f;

	unsigned int best_match = -1;
	for (size_t i = 0; i < 4; i++)
	{
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}

	return (Direction)best_match;
}
