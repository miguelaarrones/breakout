#include "ParticleGenerator.h"
#include <glad/glad.h>

ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount)
	: shader(shader), texture(texture), amount(amount)
{
	this->Init();
}

void ParticleGenerator::Update(float dt, GameObject &object, unsigned int newParticles, glm::vec2 offset)
{
	// Add new particles
	for (size_t i = 0; i < newParticles; ++i)
	{
		int unusedParticle = this->FirstUnusedParticle();
		this->RespawnParticle(this->particles[unusedParticle], object, offset);
	}

	// Update all particles
	for (size_t i = 0; i < this->amount; ++i)
	{
		Particle &p = this->particles[i];
		p.Life -= dt; // Reduce particle life

		if (p.Life > 0.0f) // Particle iss alive thus update
		{
			p.Position -= p.Velocity * dt;
			p.Color.a -= dt * 2.5f;
		}
	}
}

void ParticleGenerator::Draw()
{
	// Use additive blending to give it a 'glow' effect
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	this->shader.Use();

	for (Particle particle : this->particles)
	{
		if (particle.Life > 0.0f)
		{
			this->shader.SetVector2f("offset", particle.Position);
			this->shader.SetVector4f("color", particle.Color);
			this->shader.SetFloat("scale", particle.Scale);
			this->texture.Bind();

			glBindVertexArray(this->VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}

	// Reset to default blending mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::Init()
{
	// Setup mesh and attribute properties
	unsigned int VBO;
	float particle_quad[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(this->VAO);

	// Fill mesh buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

	// Set mesh attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glBindVertexArray(0);

	// Create this->Amount default particle instances
	for (size_t i = 0; i < this->amount; ++i)
		this->particles.push_back(Particle());
}

unsigned int ParticleGenerator::FirstUnusedParticle()
{
	// First search from last used particle, this will usually return almost instantly
	for (size_t i = LastUsedParticle; i < this->amount; ++i)
	{
		if (this->particles[i].Life <= 0.0f)
		{
			LastUsedParticle = i;
			return i;
		}
	}

	// Otherwise, do a linear search
	for (size_t i = 0; i < LastUsedParticle; ++i)
	{
		if (this->particles[i].Life <= 0.0f)
		{
			LastUsedParticle = i;
			return i;
		}
	}

	// All particles are taken, override the first one
	// Note that if it repeatedly hits this case, more particles should be reserved
	LastUsedParticle = 0;
	return 0;
}

void ParticleGenerator::RespawnParticle(Particle &particle, GameObject &object, glm::vec2 offset)
{
	float random = ((rand() % 100) - 50) / 10.0f;
	float rColor = 0.5f + ((rand() % 100) / 100.0f);
	float rScale = (rand() % 12) + 8.0f;

	particle.Position = object.Position + random + offset;
	particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
	particle.Scale = rScale;
	particle.Life = 1.0f;
	particle.Velocity = object.Velocity * 0.1f;
}
