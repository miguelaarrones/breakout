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
	
	ResourceManager::LoadTexture("assets/textures/powerup_chaos.png", true, "chaos");
	ResourceManager::LoadTexture("assets/textures/powerup_confuse.png", true, "confuse");
	ResourceManager::LoadTexture("assets/textures/powerup_increase.png", true, "increase");
	ResourceManager::LoadTexture("assets/textures/powerup_passthrough.png", true, "passthrough");
	ResourceManager::LoadTexture("assets/textures/powerup_speed.png", true, "speed");
	ResourceManager::LoadTexture("assets/textures/powerup_sticky.png", true, "sticky");

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

	SoundEngine->play2D("assets/audio/breakout.mp3", true);
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

	this->UpdatePowerUps(dt);

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

		for (PowerUp powerUp : this->PowerUps)
		{
			if (!powerUp.Destroyed)
				powerUp.Draw(*Renderer);
		}

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
				{
					box.Destroyed = true;
					this->SpawnPowerUps(box);
					SoundEngine->play2D("assets/audio/bleep.mp3", false);
				}
				else
				{
					// If block is solid, enable shake effect
					EffectsShakeTime = 0.05f;
					Effects->Shake = true;
					SoundEngine->play2D("assets/audio/solid.wav", false);
				}
				
				// Collision resolution
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);

				if (!(Ball->PassThrough && !box.IsSolid))
				{
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
					} else // Vertical collision
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

		Ball->Stuck = Ball->Sticky;

		SoundEngine->play2D("assets/audio/bleep.wav", false);
	}

	for (PowerUp &powerUp : this->PowerUps)
	{
		if (!powerUp.Destroyed)
		{
			if (powerUp.Position.y >= this->Height)
				powerUp.Destroyed = true;
			if (CheckCollision(*Player, powerUp))
			{
				ActivatePowerUp(powerUp);
				powerUp.Destroyed = true;
				powerUp.Activated = true;
				SoundEngine->play2D("assets/audio/powerup.wav", false);
			}
		}
	}
}

void Game::ActivatePowerUp(PowerUp& powerUp)
{
	if (powerUp.Type == "speed")
	{
		Ball->Velocity *= 1.2;
	} else if (powerUp.Type == "sticky")
	{
		Ball->Sticky = true;
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	} else if (powerUp.Type == "pass-through")
	{
		Ball->PassThrough = true;
		Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	} else if (powerUp.Type == "pad-size-increase")
	{
		Player->Size.x += 50;
	} else if (powerUp.Type == "confuse")
	{
		if (!Effects->Chaos)
			Effects->Confuse = true; // only activate if chaos wasn't already active
	} else if (powerUp.Type == "chaos")
	{
		if (!Effects->Confuse)
			Effects->Chaos = true;
	}
}

void Game::UpdatePowerUps(float dt)
{
	for (PowerUp& powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if (powerUp.Activated)
		{
			powerUp.Duration -= dt;

			if (powerUp.Duration <= 0.0f)
			{
				// remove powerup from list (will later be removed)
				powerUp.Activated = false;
				// deactivate effects
				if (powerUp.Type == "sticky")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
					{	// only reset if no other PowerUp of type sticky is active
						Ball->Sticky = false;
						Player->Color = glm::vec3(1.0f);
					}
				} else if (powerUp.Type == "pass-through")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
					{	// only reset if no other PowerUp of type pass-through is active
						Ball->PassThrough = false;
						Ball->Color = glm::vec3(1.0f);
					}
				} else if (powerUp.Type == "confuse")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
					{	// only reset if no other PowerUp of type confuse is active
						Effects->Confuse = false;
					}
				} else if (powerUp.Type == "chaos")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
					{	// only reset if no other PowerUp of type chaos is active
						Effects->Chaos = false;
					}
				}
			}
		}
	}
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
		[](const PowerUp& powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
	), this->PowerUps.end());
}

bool Game::IsOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type)
{
	for (const PowerUp& powerUp : powerUps)
	{
		if (powerUp.Activated)
			if (powerUp.Type == type)
				return true;
	}
	return false;
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

bool ShouldSpawn(unsigned int chance)
{
	unsigned int random = rand() % chance;
	return random == 0;
}

void Game::SpawnPowerUps(GameObject& block)
{
	if (ShouldSpawn(75))  // 1 in 75 chance
		this->PowerUps.push_back(
			PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("speed"))
		);
	if (ShouldSpawn(75))
		this->PowerUps.push_back(
			PowerUp("sticky", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("sticky"))
		);
	if (ShouldSpawn(75))
		this->PowerUps.push_back(
			PowerUp("pass-through", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("passhtrouhg"))
		);
	if (ShouldSpawn(75))
		this->PowerUps.push_back(
			PowerUp("pad-size-increase", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("increase"))
		);
	if (ShouldSpawn(15))
		this->PowerUps.push_back(
			PowerUp("confuse", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("confuse"))
		);
	if (ShouldSpawn(15))
		this->PowerUps.push_back(
			PowerUp("chaos", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("chaos"))
		);
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
