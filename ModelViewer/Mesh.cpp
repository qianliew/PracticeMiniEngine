#include "stdafx.h"
#include "Mesh.h"

Mesh::Mesh()
{
    Vertex triangleVertices[] =
    {
        { { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { -1.0f, 1.0f, 1.0f }, { 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { 1.0f, 1.0f, 1.0f }, { 0.5f, 0.5f },  { 0.0f, 0.0f, 1.0f, 1.0f } },
        { { 1.0f, -1.0f, 1.0f }, { 0.0f, 0.5f }, { 0.0f, 0.0f, 0.0f, 1.0f } },
        { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { { -1.0f, 1.0f, -1.0f }, { 0.5f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { { 1.0f, 1.0f, -1.0f }, { 1.0f, 0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { 1.0f, -1.0f, -1.0f }, { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
    };

    m_verticesSize = sizeof(triangleVertices);
    m_vertices = std::make_unique<Vertex[]>(m_verticesSize / sizeof(Vertex));
    memcpy(m_vertices.get(), &triangleVertices, m_verticesSize);

    UINT16 triangleIndices[] =
    {
        0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        4, 0, 3, 4, 3, 7
    };

    m_indicesSize = sizeof(triangleIndices);
    m_indices = std::make_unique<UINT16[]>(m_indicesSize / sizeof(UINT16));
    memcpy(m_indices.get(), &triangleIndices, sizeof(triangleIndices));
}

Mesh::~Mesh()
{
}

UINT Mesh::GetVerticesSize()
{
    return m_verticesSize;
}

UINT Mesh::GetIndicesSize()
{
    return m_indicesSize;
}

void Mesh::CopyVertices(void* destination)
{
    memcpy(destination, m_vertices.get(), m_verticesSize);
}

void Mesh::CopyIndices(void* destination)
{
    memcpy(destination, m_indices.get(), m_indicesSize);
}