#ifndef GAME_H
#define GAME_H

#include <vector>
#include "SpriteRenderer.h"
#include "GameLevel.h"
#include "BallObject.h"

enum GameState
{
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

enum Direction
{
	UP,
	RIGHT,
	DOWN,
	LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;

class Game
{
public:
	Game(unsigned int width, unsigned int height);
	~Game();
	
	GameState State;
	bool Keys[1024];
	int Width;
	int Height;
	SpriteRenderer *Renderer;

	std::vector<GameLevel> Levels;
	unsigned int currentLevel;

	GameObject *Player;
	BallObject *Ball;

	// Initial values for the player paddle
	const glm::vec2 PLAYER_SIZE;
	const float PLAYER_VELOCITY;

	// Initial values for the ball
	const float BALL_RADIUS;
	const glm::vec2 INITIAL_BALL_VELOCITY;

	// Initialize game state (load all shaders/textures/levels)
	void Init();

	// Game loop
	void ProcessInput(float dt);
	void Update(float dt);
	void Render();
	void DoCollisions();

	void ResetLevel();
	void ResetPlayer();

	// Collisions
	// AABB - AABB collision (axis-aligned bounding box)
	bool CheckCollision(GameObject &one, GameObject &two);

	// AABB - Circle collision
	Collision CheckCollision(BallObject &one, GameObject &two);

	Direction VectorDirection(glm::vec2 target);
};

#endif // !GAME_H

