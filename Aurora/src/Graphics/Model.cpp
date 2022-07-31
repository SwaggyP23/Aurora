#include "Aurorapch.h"
#include "Model.h"

namespace Aurora {

	Model::Model(const std::string& path)
	{
		Load(path);
	}

	Model::~Model()
	{
	}

	void Model::Draw()
	{
	}

	void Model::Load(const std::string& path)
	{
	}

	void Model::InsertVertex(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::unordered_map<IndexSet, int32_t>& mapping, VertexSet& inputVertices, IndexSet& indexSet)
	{
	}

}