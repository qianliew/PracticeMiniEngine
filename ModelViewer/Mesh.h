#pragma once

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 texCoord;
    XMFLOAT4 color;
};

class Mesh
{
private:
	XMVECTOR m_position;
    std::unique_ptr<Vertex[]> m_vertices;
    std::unique_ptr<UINT16[]> m_indices;
    UINT m_verticesSize;
    UINT m_indicesSize;

public:
    Mesh();
    
    ~Mesh();

    UINT GetVerticesSize();
    UINT GetIndicesSize();
    void CopyVertices(void* destination);
    void CopyIndices(void* destination);
};