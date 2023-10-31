#include "stdafx.h"
#include "Material.h"

void Material::AddTexture(UINT id)
{
	if (textureList.size() < maxTextureNum)
	{
		textureList.push_back(id);
	}
}
