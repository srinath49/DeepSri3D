#pragma once

#ifndef _MESH_H_
#define _MESH_H_

//---------------------------------------------------------------------------------------------------
#include "VertexData.hpp"
#include <vector>

//---------------------------------------------------------------------------------------------------
class Mesh
{
public:
	Mesh();
	~Mesh();

	void LoadMesh(const std::string& meshPath);
	void InitializeMesh();

private:
	std::vector<Vertex>		m_vertices;
	std::vector<uint32_t>	m_indices;
	uint16_t				m_vertexBufferId;
	uint16_t				m_indexBufferId;
};
#endif // !_MESH_H_

