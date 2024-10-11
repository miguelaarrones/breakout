#ifndef BALL_OBJECT_H
#define BALL_OBJECT_H

#include <glm/glm.hpp>

#include "GameObject.h"
#include "Texture.h"

class BallObject : public GameObject
{
public:
	BallObject();
	BallObject(glm::vec2 pos, float radius , glm::vec2 velocity, Texture2D sprite);

	glm::vec2 Move(float dt, unsigned int window_width);
	void Reset(glm::vec2 position, glm::vec2 velocity);

	// Ball state
	float Radius;
	bool Stuck;

};

#endif // !BALL_OBJECT_H
