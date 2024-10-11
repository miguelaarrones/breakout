#ifndef GAME_H
#define GAME_H

enum GameState
{
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

class Game
{
public:
	Game(unsigned int width, unsigned int height);
	~Game();
	
	GameState State;
	bool Keys[1024];

	// Initialize game state (load all shaders/textures/levels)
	void Init();

	// Game loop
	void ProcessInput(float dt);
	void Update(float dt);
	void Render();
};

#endif // !GAME_H

