#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H

#include <glm/glm.hpp>
#include "Texture.h"
#include "Shader.h"
#include "GameObject.h"
#include <vector>

struct Particle
{
	glm::vec2 Position, Velocity;
	glm::vec4 Color;
	float Scale;
	float Life;

	Particle()
		: Position(0.0f), Velocity(0.0f), Color(1.0f), Scale(10.0f), Life(0.0f)
	{
	}
};

class ParticleGenerator
{
public:
	ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);

	void Update(float dt, GameObject &object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f));
	void Draw();

private:
	std::vector<Particle> particles;
	unsigned int amount;

	// Stores the index of the last particle used (for quick access to next dead particle)
	unsigned int LastUsedParticle = 0;

	Shader shader;
	Texture2D texture;
	unsigned int VAO;

	// Initializes buffer and vertex attributes
	void Init();

	// Returns the first Particle index that's currently unused
	// e.g. Life <= 0.0f or 0 if no particle is currently inactive
	unsigned int FirstUnusedParticle();

	void RespawnParticle(Particle &particle, GameObject &object, glm::vec2 offset = glm::vec2(0.0f));
};

#endif // !PARTICLE_GENERATOR_H