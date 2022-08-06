//#pragma once
//
//// This is to be continued when reworking the shader to system to use Spir-V and Uniform Buffers that way meshes and materials are easier
//
//#include "Core/Base.h"
//#include "Buffer.h"
//#include "VertexArray.h"
//
//#include <glm/glm.hpp>
//
//namespace Aurora {
//
//	struct Vertex
//	{
//		glm::vec3 Position;
//		glm::vec3 Normal;
//		glm::vec2 TexCoords;
//	};
//
//	class Mesh
//	{
//	public:
//		Mesh(const Ref<VertexArray>& vao, const Ref<IndexBuffer>& ibo/*TODO: material*/);
//		Mesh(const Mesh& other);
//		~Mesh();
//
//		void Draw();
//
//	private:
//		Ref<VertexArray> m_VertexArray; // This will need to include a vertex buffer and an index buffer
//		Ref<IndexBuffer> m_IndexBuffer;
//		// TODO: m_Material
//
//	};
//
//}
#ifndef MESH_H
#define MESH_H

#include "Graphics/Shader.h"
#include "Renderer/RenderCommand.h"

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

namespace Aurora {

    struct Vertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;
        // tangent
        glm::vec3 Tangent;
        // bitangent
        glm::vec3 Bitangent;
        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
    };

    struct TextureMesh {
        unsigned int id;
        std::string type;
        std::string path;
    };

    class Mesh {
    public:
        // mesh Data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<TextureMesh>      textures;
        unsigned int VAO;

        // constructor
        Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<TextureMesh> textures);

        // render the mesh
        void Draw(Aurora::Shader& shader);

    private:
        // render data 
        uint32_t VBO, EBO;

        // initializes all the buffer objects/arrays
        void setupMesh();
    };

}
#endif