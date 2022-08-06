#pragma once

#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"

#include <string>
#include <vector>

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

namespace Aurora {

    class Model
    {
    public:
        // model data 
        std::vector<TextureMesh> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
        std::vector<Mesh>    meshes;
        std::string directory;
        bool gammaCorrection;

        Model() = default;
        // constructor, expects a filepath to a 3D model.
        Model(std::string path, bool gamma = false);

        // draws the model, and thus all its meshes
        void Draw(Aurora::Shader& shader);

    private:
        // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
        void loadModel(std::string& path);

        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
        void processNode(aiNode* node, const aiScene* scene);

        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

        // checks all material textures of a given type and loads the textures if they're not loaded yet.
        // the required info is returned as a Texture struct.
		std::vector<TextureMesh> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

    };

}

#endif
/*

#include "Core/Base.h"
#include "Texture.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Aurora {

	class Model
	{
	public:
		Model() = default;
		Model(const std::string& path/*material);
		~Model();

		void Draw();

		inline const std::vector<Mesh>& GetMesh() const { return m_Meshes; }
		
	private:
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	private:
		std::vector<Mesh> m_Meshes;
		std::vector<Texture> m_Textures;
		std::string m_Directory;

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

}*/