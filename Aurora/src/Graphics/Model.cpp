#include "Aurorapch.h"
#include "Model.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Aurora {

    static uint32_t TextureFromFile(const std::string& path, const std::string& directory)
    {
        std::string filename = path;
        filename = directory + '/' + filename;

        uint32_t textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        Byte* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            AR_CORE_ERROR_TAG("TempModel", "Texture failed to load at path: {0}", path);
            stbi_image_free(data);
        }

        return textureID;
    }

    Model::Model(std::string path, bool gamma)
        : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    void Model::Draw(Aurora::Shader & shader)
    {
        for (uint32_t i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    void Model::loadModel(std::string& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            AR_CORE_ERROR_TAG("ERROR::ASSIMP", "->{0}", importer.GetErrorString());
            return;
        }
        // retrieve the directory path of the filepath
        std::replace(path.begin(), path.end(), '\\', '/');
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    void Model::processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<TextureMesh> textures;

        // walk through each of the mesh's vertices
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (uint32_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        std::vector<TextureMesh> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<TextureMesh> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<TextureMesh> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<TextureMesh> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    std::vector<TextureMesh> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<TextureMesh> textures;
        for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (uint32_t j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                TextureMesh texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

}

//namespace Aurora {
//
//	Model::Model(const std::string& path)
//	{
//		Load(path);
//	}
//
//	Model::~Model()
//	{
//	}
//
//	void Model::Draw()
//	{
//		for (auto& mesh : m_Meshes)
//			mesh.Draw();
//	}
//
//	void Model::processNode(aiNode* node, const aiScene* scene)
//	{
//		// process each mesh located at the current node
//		for (unsigned int i = 0; i < node->mNumMeshes; i++)
//		{
//			// the node object only contains indices to index the actual objects in the scene. 
//			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
//			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//			m_Meshes.push_back(processMesh(mesh, scene));
//		}
//		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
//		for (unsigned int i = 0; i < node->mNumChildren; i++)
//		{
//			processNode(node->mChildren[i], scene);
//		}
//
//	}
//
//    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
//    {
//        // data to fill
//        std::vector<Vertex> vertices;
//        std::vector<unsigned int> indices;
//        std::vector<Texture> textures;
//
//        // walk through each of the mesh's vertices
//        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
//        {
//            Vertex vertex;
//            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
//            // positions
//            vector.x = mesh->mVertices[i].x;
//            vector.y = mesh->mVertices[i].y;
//            vector.z = mesh->mVertices[i].z;
//            vertex.Position = vector;
//            // normals
//            if (mesh->HasNormals())
//            {
//                vector.x = mesh->mNormals[i].x;
//                vector.y = mesh->mNormals[i].y;
//                vector.z = mesh->mNormals[i].z;
//                vertex.Normal = vector;
//            }
//            // texture coordinates
//            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
//            {
//                glm::vec2 vec;
//                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
//                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
//                vec.x = mesh->mTextureCoords[0][i].x;
//                vec.y = mesh->mTextureCoords[0][i].y;
//                vertex.TexCoords = vec;
//                // tangent
//                //vector.x = mesh->mTangents[i].x;
//                //vector.y = mesh->mTangents[i].y;
//                //vector.z = mesh->mTangents[i].z;
//                //vertex.Tangent = vector;
//                //// bitangent
//                //vector.x = mesh->mBitangents[i].x;
//                //vector.y = mesh->mBitangents[i].y;
//                //vector.z = mesh->mBitangents[i].z;
//                //vertex.Bitangent = vector;
//            }
//            else
//                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
//
//            vertices.push_back(vertex);
//        }
//        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
//        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
//        {
//            aiFace face = mesh->mFaces[i];
//            // retrieve all indices of the face and store them in the indices vector
//            for (unsigned int j = 0; j < face.mNumIndices; j++)
//                indices.push_back(face.mIndices[j]);
//        }
//        // process materials
//        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
//        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
//        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
//        // Same applies to other texture as the following list summarizes:
//        // diffuse: texture_diffuseN
//        // specular: texture_specularN
//        // normal: texture_normalN
//
//        // 1. diffuse maps
//        //vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
//        //textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//        //// 2. specular maps
//        //vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
//        //textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//        //// 3. normal maps
//        //std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
//        //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
//        //// 4. height maps
//        //std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
//        //textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
//
//        // return a mesh object created from the extracted mesh data
//        Ref<VertexArray> vao = VertexArray::Create();
//        Ref<VertexBuffer> vbo;
//        Ref<IndexBuffer> ibo = IndexBuffer::Create(&indices[0], indices.size());
//        vbo->SetLayout({
//            { ShaderDataType::Float3, "a_Position"  },
//            { ShaderDataType::Float3, "a_Normals"   },
//            { ShaderDataType::Float2, "a_TexCoords" }
//        });
//        vbo->SetData(vertices.data(), vertices.size());
//        vao->AddVertexBuffer(vbo);
//
//        return Mesh(vao, ibo/*, textures*/);
//    }
//
//	void Model::Load(const std::string& path)
//	{
//		Assimp::Importer importer;
//		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
//		// check for errors
//		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
//		{
//            AR_CORE_ERROR("ERROR::ASSIMP:: {}", importer.GetErrorString());
//			return;
//		}
//		// retrieve the directory path of the filepath
//		m_Directory = path.substr(0, path.find_last_of('/'));
//
//		// process ASSIMP's root node recursively
//		processNode(scene->mRootNode, scene);
//	}
//
//	void Model::InsertVertex(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::unordered_map<IndexSet, int32_t>& mapping, VertexSet& inputVertices, IndexSet& indexSet)
//	{
//	}
//
//}