#pragma once

class Material
{
private:
	// TODO: Parse the shader to decide how many textures in each material.
	// Material is equal to Shader in this engine right now, and there is just one object shader.
	const UINT maxTextureNum = 1;

	std::vector<UINT> textureList;

public:
	void AddTexture(UINT id);

	inline const UINT GetTextureIDAt(UINT index) const { return textureList[index]; }
};
