#pragma once

#include "Core/Base.h"
#include "Mesh.h"

namespace Aurora {

	class Model
	{
	public:
		Model(const std::string& path/*material*/);
		~Model();

		void Draw();

		inline const Ref<Mesh>& GetMesh() const { return m_Mesh; }

	private:
		Ref<Mesh> m_Mesh;

		struct VertexSet
		{
			std::vector<glm::vec3> positions;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> texCoords;
		};

		struct IndexSet
		{
			uint32_t position;
			uint32_t uv;
			uint32_t normal;

			bool operator==(const IndexSet& other) const
			{
				return position == other.position && uv == other.uv && normal == other.normal;
			}
		};

		friend struct std::hash<IndexSet>;

		void Load(const std::string& path);
		void InsertVertex(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::unordered_map<IndexSet, int32_t>& mapping, VertexSet& inputVertices, IndexSet& indexSet);

	};

}