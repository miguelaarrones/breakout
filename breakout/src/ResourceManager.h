#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <string>

#include "Shader.h"
#include "Texture.h"


// A static singleton ResourceManager class that hosts several
// functions to load Textures and Shaders. Each loaded texture
// and/or shader is also stored for future reference by string
// handles. All functions and resources are static and no 
// public constructor is defined.
class ResourceManager
{
public:
	// Resource storage
	static std::map<std::string, Shader> Shaders;
	static std::map<std::string, Texture2D> Textures;

	// Loads (and generates) a shader program from file loading vertex, fragment (and geometry)
	// shader's source code. If gShader is not nullptr, it also loads a geometry shader
	static Shader LoadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, std::string name);

	// Retrieves a stored Shader
	static Shader GetShader(std::string name);

	// Loads (and generates) a texture from file
	static Texture2D LoadTexture(const char *file, bool alpha, std::string name);

	// Retrieves a stored texture
	static Texture2D GetTexture(std::string name);

	// Properly de-allocates all loaded resources
	static void Clear();
private:
	ResourceManager() = default;

	// Loads and generates a shader from file
	static Shader loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile = nullptr);
	
	// Loads a single texture from file
	static Texture2D loadTextureFromFile(const char *file, bool alpha);
};

#endif // !RESOURCE_MANAGER_H

