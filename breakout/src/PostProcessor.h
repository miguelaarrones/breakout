#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include "Texture.h"
#include "Shader.h"

class PostProcessor
{
public:
	Shader PostProcessingShader;
	Texture2D Texture;
	unsigned int Width, Height;

	// Options
	bool Confuse, Chaos, Shake;

	PostProcessor(Shader shader, unsigned int width, unsigned int height);

	// Prepares the PostProcessor's frame buffer operations before rendering the game
	void BeginRender();

	// Should be called after rendering the game, so it stores all the rendered data into a texture object
	void EndRender();

	// Renders the PostProcessor texture quad (as a screen-encompassing large sprite)
	void Render(float time);

private:
	// Render state
	unsigned int MSFBO, FBO; // MSFBO = Multi-sampled FBO. FBO is regular, used for blitting MS color-buffer to texture
	unsigned int RBO; // RBO is used for multi-sampled color buffer
	unsigned int VAO;

	// Initialize quad for rendering PostProcessing texture
	void InitRenderData();
};

#endif // !POST_PROCESSOR_H