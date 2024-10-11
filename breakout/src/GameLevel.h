#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include <vector>

#include "GameObject.h"
#include "SpriteRenderer.h"

class GameLevel
{
public:
	GameLevel() = default;

	// Level state
	std::vector<GameObject> Bricks;

	// Loads level from file
	void Load(const char *file, unsigned int levelWidth, unsigned int levelHeight);

	// Render level
	void Draw(SpriteRenderer &renderer);

	// Check if level is completed (all non-solid tiles are destroyed)
	bool IsCompleted();

private:
	// Initialize level from tile data
	void Init(std::vector<std::vector<unsigned int>> tileData,
			  unsigned int levelWidth, unsigned int levelHeight);
};

#endif // !GAME_LEVEL_H

