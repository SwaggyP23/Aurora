#include "Aurorapch.h"
#include "Mesh.h"

#include "Renderer/Renderer.h"
#include "Utils/AssimpLogStream.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace Aurora {

    namespace Utils {

        glm::mat4 Mat4FromAIMatrix4x4(const aiMatrix4x4& matrix)
        {
            glm::mat4 result;
            // The a, b, c, d in assimp is the row and the 1, 2, 3, 4 is the columns...
            result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
            result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
            result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
            result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
            return result;
        }
    }

    constexpr uint32_t s_MeshImportFlags =
        aiProcess_CalcTangentSpace       | // Create Binormals/tangents just in case
        aiProcess_Triangulate            | // Make sure we're triangles
        aiProcess_SortByPType            | // Splite meshes by primitive type
        aiProcess_GenNormals             | // Make sure we have legit normals
        aiProcess_GenUVCoords            | // Convert UVs if required
        aiProcess_OptimizeMeshes         | // Batch draws where possible
        aiProcess_OptimizeGraph          |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_ValidateDataStructure  | // Validation
        aiProcess_GlobalScale            ; // For example: convert cm to m for fbx import (and other formats where cm is native)

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////// MeshSource
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    MeshSource::MeshSource(const std::filesystem::path& filePath)
    {
        AssimpLogStream::Init();

        AR_CORE_INFO_TAG("Mesh", "Loading mesh from: {0}", filePath.string());

        m_Importer = CreateScope<Assimp::Importer>();
        m_Importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

        const aiScene* scene = m_Importer->ReadFile(filePath.string(), s_MeshImportFlags);
        if (!scene)
        {
            AR_CORE_ERROR_TAG("Mesh", "Failed to load mesh from: {0}", filePath.string());
            return;
        }

        m_Scene = scene;

        if (!scene->HasMeshes())
            return;

        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;

        // Setting it to the float bound so that when comparing min max for the first time in the loop it works
        m_BoundingBox.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
        m_BoundingBox.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        m_SubMeshes.reserve(scene->mNumMeshes);
        for (uint32_t idx = 0; idx < scene->mNumMeshes; idx++)
        {
            aiMesh* mesh = scene->mMeshes[idx];

            SubMesh& subMesh = m_SubMeshes.emplace_back();
            subMesh.BaseVertex = vertexCount;
            subMesh.BaseIndex = indexCount;
            subMesh.MaterialIndex = mesh->mMaterialIndex;
            subMesh.VertexCount = mesh->mNumVertices;
            subMesh.IndexCount = mesh->mNumFaces * 3; // Since we're in triangles
            subMesh.MeshName = mesh->mName.C_Str();

            vertexCount += mesh->mNumVertices;
            indexCount += subMesh.IndexCount;

            AR_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");
            AR_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");

            // Vertices...
            // Calc bounding box by going through all the vertices and updating if the position is higher or lower
            AABB& aabb = subMesh.BoundingBox;
            aabb.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
            aabb.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
            for (size_t i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
                vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
                aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
                aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
                aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
                aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
                aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
                aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);

                if (mesh->HasTangentsAndBitangents())
                {
                    vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
                    vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
                }

                if (mesh->HasTextureCoords(0))
                    vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

                m_Vertices.push_back(vertex);
            }

            // Indices...
            for (size_t i = 0; i < mesh->mNumFaces; i++)
            {
                AR_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices since we're in triangles");
                Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
                m_Indices.push_back(index);

                m_TriangleCache[idx].emplace_back(
                    m_Vertices[index.V1 + subMesh.BaseVertex],
                    m_Vertices[index.V2 + subMesh.BaseVertex],
                    m_Vertices[index.V3 + subMesh.BaseVertex]);
            }
        }

#if 0 // MESH_DEBUG_LOG
        HZ_CORE_INFO_TAG("Mesh", "Traversing nodes for scene '{0}'", filename);
        Utils::PrintNode(scene->mRootNode, 0);
#endif

        TraverseNodes(scene->mRootNode);

        // Calculate the bounding for the whole mesh by going throught the submeshes
        for (const auto& subMesh : m_SubMeshes)
        {
            AABB transformedSubmeshAABB = subMesh.BoundingBox;

            glm::vec3 min = glm::vec3(subMesh.Transform * glm::vec4(transformedSubmeshAABB.Min, 1.0f));
            glm::vec3 max = glm::vec3(subMesh.Transform * glm::vec4(transformedSubmeshAABB.Max, 1.0f));

            m_BoundingBox.Min.x = glm::min(m_BoundingBox.Min.x, min.x);
            m_BoundingBox.Min.y = glm::min(m_BoundingBox.Min.y, min.y);
            m_BoundingBox.Min.z = glm::min(m_BoundingBox.Min.z, min.z);
            m_BoundingBox.Max.x = glm::max(m_BoundingBox.Max.x, max.x);
            m_BoundingBox.Max.y = glm::max(m_BoundingBox.Max.y, max.y);
            m_BoundingBox.Max.z = glm::max(m_BoundingBox.Max.z, max.z);
        }

        // Materials...
        // PBR STUFF
#if 1
        Ref<Texture2D> whiteTexture = Renderer::GetWhiteTexture();
        if (scene->HasMaterials())
        {
            AR_CORE_TRACE_TAG("Mesh", "Material for: {0}", filePath.string());

            m_Materials.resize(scene->mNumMaterials);

            for (size_t i = 0; i < scene->mNumMaterials; i++)
            {
                aiMaterial* aiMaterial = scene->mMaterials[i];
                aiString aiMaterialName = aiMaterial->GetName();
                Ref<Material> mat = Material::Create(aiMaterialName.data, Renderer::GetShaderLibrary()->Get("AuroraPBRStatic"));
                m_Materials[i] = mat;
            
                AR_CORE_TRACE_TAG("Mesh", "\t{0} (Index = {1})", aiMaterialName.data, i);
                aiString aiTexturePath;
                uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
                AR_CORE_TRACE_TAG("Mesh", "\tTexture Count: {0}", textureCount);

                glm::vec3 albedoColor = glm::vec3(0.8f);
                aiColor3D aiColor;
                if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == aiReturn_SUCCESS)
                    albedoColor = { aiColor.r, aiColor.g, aiColor.b };

                mat->Set("u_MaterialUniforms.AlbedoColor", albedoColor);

                float shininess;
                float metalness;
                float roughness;
                if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
                    shininess = 80.0f; // Default value

                if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
                    metalness = 0.0f;

                roughness = 1.0f - glm::sqrt(shininess / 100.0f);

                AR_CORE_TRACE_TAG("Mesh", "\tColor: {0} - {1} - {2}", albedoColor.r, albedoColor.g, albedoColor.b);
                AR_CORE_TRACE_TAG("Mesh", "\tRoughness: {0}", roughness);
                AR_CORE_TRACE_TAG("Mesh", "\tMetalness: {0}", metalness);

                // Albedo maps...
                bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexturePath) == aiReturn_SUCCESS;
                bool fallback = !hasAlbedoMap;
                if (hasAlbedoMap)
                {
                    Ref<Texture2D> albedoTexture;
                    TextureProperties props = {};
                    props.DebugName = aiTexturePath.C_Str();
                    props.SRGB = true;
                    const aiTexture* aiTextureEmbedded = scene->GetEmbeddedTexture(aiTexturePath.C_Str());
                    if (aiTextureEmbedded)
                    {
                        albedoTexture = Texture2D::Create(ImageFormat::RGB,
                            aiTextureEmbedded->mWidth,
                            aiTextureEmbedded->mHeight,
                            aiTextureEmbedded->pcData,
                            props);
                    }
                    else
                    {
                        std::filesystem::path parentPath = filePath.parent_path();
                        parentPath /= std::string(aiTexturePath.data);
                        std::string texturePath = parentPath.string();
                        AR_CORE_TRACE_TAG("Mesh", "\tAlbedo Map Path: {0}", texturePath);
                        albedoTexture = Texture2D::Create(texturePath, props);
                    }

                    if (albedoTexture && albedoTexture->IsLoaded())
                    {
                        mat->Set("u_AlbedoTexture", albedoTexture);
                        mat->Set("u_MaterialUniforms.AlbedoColor", glm::vec3(1.0f));
                    }
                    else
                    {
                        AR_CORE_ERROR_TAG("Mesh", "Failed to load texture from: {0}", aiTexturePath.C_Str());
                        fallback = true;
                    }
                }

                if (fallback)
                {
                    AR_CORE_TRACE_TAG("Mesh", "\tNo Albedo Texture");
                    mat->Set("u_AlbedoTexture", whiteTexture);
                }

                // Normals maps...
                bool hasNormalMap = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexturePath) == aiReturn_SUCCESS;
                fallback = !hasNormalMap;
                if (hasNormalMap)
                {
                    Ref<Texture2D> normalMap;
                    TextureProperties props = {};
                    props.DebugName = aiTexturePath.C_Str();
                    const aiTexture* aiTextureEmbedded = scene->GetEmbeddedTexture(aiTexturePath.C_Str());
                    if (aiTextureEmbedded)
                    {
                        normalMap = Texture2D::Create(ImageFormat::RGB, 
                            aiTextureEmbedded->mWidth, 
                            aiTextureEmbedded->mHeight, 
                            aiTextureEmbedded->pcData, 
                            props);
                    }
                    else
                    {
                        std::filesystem::path parentPath = filePath.parent_path();
                        parentPath /= std::string(aiTexturePath.data);
                        std::string texturePath = parentPath.string();
                        AR_CORE_TRACE_TAG("Mesh", "\tNomal Map path: {0}", texturePath);
                        normalMap = Texture2D::Create(texturePath);
                    }

                    if (normalMap && normalMap->IsLoaded())
                    {
                        mat->Set("u_NormalTexture", normalMap);
                        mat->Set("u_MaterialUniforms.UseNormalMap", true);
                    }
                    else
                    {
                        AR_CORE_ERROR_TAG("Mesh", "\tCould not load texture from: {0}", aiTexturePath.C_Str());
                        fallback = true;
                    }
                }

                if (fallback)
                {
                    AR_CORE_TRACE_TAG("Mesh", "\tNo Normal Map");
                    mat->Set("u_NormalTexture", whiteTexture);
                    mat->Set("u_MaterialUniforms.UseNormalMap", false);
                }

                // Roughness maps...
                bool hasRoughnessMap = aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexturePath) == aiReturn_SUCCESS;
                fallback = !hasRoughnessMap;
                if (hasRoughnessMap)
                {
                    Ref<Texture2D> roughnessMap;
                    TextureProperties props = {};
                    props.DebugName = aiTexturePath.C_Str();
                    const aiTexture* aiTextureEmbedded = scene->GetEmbeddedTexture(aiTexturePath.C_Str());
                    if (aiTextureEmbedded)
                    {
                        roughnessMap = Texture2D::Create(ImageFormat::RGB,
                            aiTextureEmbedded->mWidth,
                            aiTextureEmbedded->mHeight,
                            aiTextureEmbedded->pcData,
                            props);
                    }
                    else
                    {
                        std::filesystem::path parentPath = filePath.parent_path();
                        parentPath /= std::string(aiTexturePath.data);
                        std::string texturePath = parentPath.string();
                        AR_CORE_TRACE_TAG("Mesh", "\tRoughness Map path: {0}", texturePath);
                        roughnessMap = Texture2D::Create(texturePath);
                    }

                    if (roughnessMap && roughnessMap->IsLoaded())
                    {
                        mat->Set("u_RoughnessTexture", roughnessMap);
                        mat->Set("u_MaterialUniforms.Roughness", 1.0f);
                    }
                    else
                    {
                        AR_CORE_ERROR_TAG("Mesh", "\tCould not load texture from: {0}", aiTexturePath.C_Str());
                        fallback = true;
                    }
                }

                if (fallback)
                {
                    AR_CORE_TRACE_TAG("Mesh", "\tNo Roughness Map");
                    mat->Set("u_RoughnessTexture", whiteTexture);
                    mat->Set("u_MaterialUniforms.Roughness", roughness);
                }

                // Metalness maps...
#if HAZEL_WAY
                bool hasMetalnessMap = false;
                for (size_t p = 0; p < aiMaterial->mNumProperties; p++)
                {
                    aiMaterialProperty* prop = aiMaterial->mProperties[p];

                    if (prop->mType == aiPTI_String)
                    {
                        uint32_t strLength = *(uint32_t*)prop->mData;
                        std::string str(prop->mData + 4, strLength);

                        std::string key = prop->mKey.data;
                        if (key == "$raw.ReflectionFactor|file")
                        {
                            Ref<Texture2D> metalnessMap;
                            TextureProperties props = {};
                            props.DebugName = str;
                            const aiTexture* aiTextureEmbedded = scene->GetEmbeddedTexture(str.data());
                            if (aiTextureEmbedded)
                            {
                                metalnessMap = Texture2D::Create(ImageFormat::RGB,
                                    aiTextureEmbedded->mWidth, 
                                    aiTextureEmbedded->mHeight, 
                                    aiTextureEmbedded->pcData,
                                    props);
                            }
                            else
                            {
                                std::filesystem::path parentPath = filePath.parent_path();
                                parentPath /= str;
                                std::string texturePath = parentPath.string();
                                AR_CORE_TRACE_TAG("Mesh", "\tMetalness map path = {0}", texturePath);
                                metalnessMap = Texture2D::Create(texturePath);
                            }

                            if (metalnessMap && metalnessMap->IsLoaded())
                            {
                                hasMetalnessMap = true;
                                mat->Set("u_MetalnessTexture", metalnessMap);
                                mat->Set("u_MaterialUniforms.Metalness", 1.0f);
                            }
                            else
                            {
                                AR_CORE_ERROR_TAG("Mesh", "\tCould not load texture from: {0}", str);
                            }
                            break;
                        }
                    }
                }

                fallback = !hasMetalnessMap;
                if (fallback)
                {
                    AR_CORE_TRACE_TAG("Mesh", "\tNo Metalness Map");
                    mat->Set("u_MetalnessTexture", whiteTexture);
                    mat->Set("u_MaterialUniforms.Metalness", metalness);
                }
#else
                bool hasMetalnessMap = aiMaterial->GetTexture(aiTextureType_METALNESS, 0, &aiTexturePath) == aiReturn_SUCCESS;
                fallback = !hasMetalnessMap;
                if (hasMetalnessMap)
                {
                    Ref<Texture2D> metalnessMap;
                    TextureProperties props = {};
                    props.DebugName = aiTexturePath.C_Str();
                    const aiTexture* aiTextureEmbedded = scene->GetEmbeddedTexture(aiTexturePath.C_Str());
                    if (aiTextureEmbedded)
                    {
                        metalnessMap = Texture2D::Create(ImageFormat::RGB, 
                            aiTextureEmbedded->mWidth, 
                            aiTextureEmbedded->mHeight, 
                            aiTextureEmbedded->pcData, 
                            props);
                    }
                    else
                    {
                        std::filesystem::path parentPath = filePath.parent_path();
                        parentPath /= std::string(aiTexturePath.data);
                        std::string texturePath = parentPath.string();
                        AR_CORE_TRACE_TAG("Mesh", "\tRoughness Map path: {0}", texturePath);
                        metalnessMap = Texture2D::Create(texturePath);
                    }

                    if (metalnessMap && metalnessMap->IsLoaded())
                    {
                        mat->Set("u_MetalnessTexture", metalnessMap);
                        mat->Set("u_MaterialUniforms.Metalness", 1.0f);
                    }
                    else
                    {
                        AR_CORE_ERROR_TAG("Mesh", "\tCould not load texture from: {0}", aiTexturePath.C_Str());
                        fallback = true;
                    }
                }

                if (fallback)
                {
                    AR_CORE_TRACE_TAG("Mesh", "\tNo Metalness Map");
                    mat->Set("u_MetalnessTexture", whiteTexture);
                    mat->Set("u_MaterialUniforms.Metalness", metalness);
                }
#endif //! Hazel way
            }
            AR_CORE_TRACE_TAG("Mesh", "------------------------------");
        }
        else
        {
            Ref<Material> mat = Material::Create("Aurora-Default", Renderer::GetShaderLibrary()->Get("AuroraPBRStatic"));
            mat->Set("u_MaterialUniforms.AlbedoColor", glm::vec3(0.8f));
            mat->Set("u_MaterialUniforms.Metalness", 0.0f);
            mat->Set("u_MaterialUniforms.Roughness", 0.8f);
            mat->Set("u_MaterialUniforms.UseNormalMap", false);

            mat->Set("u_AlbedoTexture", whiteTexture);
            mat->Set("u_MetalnessTexture", whiteTexture);
            mat->Set("u_RoughnessTexture", whiteTexture);
            m_Materials.push_back(mat);
        }
#endif

        m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), (uint32_t)(m_Vertices.size() * sizeof(Vertex)), BufferUsage::Dynamic);
        m_VertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal"   },
            { ShaderDataType::Float3, "a_Tangent"  },
            { ShaderDataType::Float3, "a_Binormal" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });

        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)(m_Indices.size() * sizeof(Index)));

        m_VertexArray = VertexArray::Create();
        m_VertexArray->AddVertexBuffer(m_VertexBuffer);
        m_VertexArray->SetIndexBuffer(m_IndexBuffer);
    }

    MeshSource::MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform)
        : m_Vertices(vertices), m_Indices(indices)
    {
        SubMesh subMesh = {};
        subMesh.BaseVertex = 0;
        subMesh.BaseIndex = 0;
        subMesh.IndexCount = (uint32_t)indices.size() * 3;
        subMesh.Transform = transform;
        m_SubMeshes.push_back(subMesh);

        m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), (uint32_t)(m_Vertices.size() * sizeof(Vertex), BufferUsage::Dynamic));
        m_VertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal"   },
            { ShaderDataType::Float3, "a_Tangent"  },
            { ShaderDataType::Float3, "a_Binormal" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });

        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)(m_Indices.size() * sizeof(Index)));

        m_VertexArray = VertexArray::Create();
        m_VertexArray->AddVertexBuffer(m_VertexBuffer);
        m_VertexArray->SetIndexBuffer(m_IndexBuffer);
    }

    MeshSource::MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const std::vector<SubMesh>& subMeshes)
        : m_Vertices(vertices), m_Indices(indices), m_SubMeshes(subMeshes)
    {
        m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), (uint32_t)(m_Vertices.size() * sizeof(Vertex), BufferUsage::Dynamic));
        m_VertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal"   },
            { ShaderDataType::Float3, "a_Tangent"  },
            { ShaderDataType::Float3, "a_Binormal" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });

        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)(m_Indices.size() * sizeof(Index)));

        m_VertexArray = VertexArray::Create();
        m_VertexArray->AddVertexBuffer(m_VertexBuffer);
        m_VertexArray->SetIndexBuffer(m_IndexBuffer);
    }

    MeshSource::~MeshSource()
    {
        AssimpLogStream::Shutdown();
    }

    void MeshSource::DumpVertexBuffer()
    {
        // TODO: Convert to be displayed inside ImGui...
        AR_CORE_TRACE_TAG("Mesh", "------------------------------------------------------");
        AR_CORE_TRACE_TAG("Mesh", "Vertex Buffer Dump");
        
        AR_CORE_TRACE_TAG("Mesh", "Mesh: {0}", m_AssetPath.string());
        for (size_t i = 0; i < m_Vertices.size(); i++)
        {
            const Vertex& vertex = m_Vertices[i];
            AR_CORE_TRACE_TAG("Mesh", "Vertex: {0}", i);
            AR_CORE_TRACE_TAG("Mesh", "Position: {0} - {1} - {2}", vertex.Position.x, vertex.Position.y, vertex.Position.z);
            AR_CORE_TRACE_TAG("Mesh", "Normal: {0} - {1} - {2}", vertex.Normal.x, vertex.Normal.y, vertex.Normal.z);
            AR_CORE_TRACE_TAG("Mesh", "Binormal: {0} - {1} - {2}", vertex.Binormal.x, vertex.Binormal.y, vertex.Binormal.z);
            AR_CORE_TRACE_TAG("Mesh", "Tangent: {0} - {1} - {2}", vertex.Tangent.x, vertex.Tangent.y, vertex.Tangent.z);
            AR_CORE_TRACE_TAG("Mesh", "TexCoord: {0} - {1}", vertex.TexCoords.x, vertex.TexCoords.y);
        }

        AR_CORE_TRACE_TAG("Mesh", "------------------------------------------------------");
    }

    void MeshSource::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, uint32_t level)
    {
        glm::mat4 localTransform = Utils::Mat4FromAIMatrix4x4(node->mTransformation);
        glm::mat4 transform = parentTransform * localTransform;
        m_NodeMap[node].resize(node->mNumMeshes);
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            uint32_t mesh = node->mMeshes[i];
            SubMesh& subMesh = m_SubMeshes[mesh];
            subMesh.NodeName = node->mName.C_Str();
            subMesh.Transform = transform;
            subMesh.LocalTransform = localTransform;
            m_NodeMap[node][i] = mesh;
        }

        // AR_CORE_TRACE_TAG("Mesh", "Depth: {0} - {1}", level, node->mName.C_Str());

        for (uint32_t i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], transform, level++);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////// StaticMesh
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    StaticMesh::StaticMesh(Ref<MeshSource> meshSource)
        : m_MeshSource(meshSource)
    {
        SetSubMeshes({});

        const std::vector<Ref<Material>>& meshMats = meshSource->GetMaterials();
        uint32_t numMats = (uint32_t)meshMats.size();
        m_MaterialsTable = MaterialTable::Create(numMats);
        for (uint32_t i = 0; i < numMats; i++)
            m_MaterialsTable->SetMaterial(i, MaterialAsset::Create(meshMats[i]));
    }

    StaticMesh::StaticMesh(Ref<MeshSource> meshSource, const std::vector<uint32_t>& subMeshes)
        : m_MeshSource(meshSource)
    {
        SetSubMeshes(subMeshes);

        const std::vector<Ref<Material>>& meshMats = meshSource->GetMaterials();
        uint32_t numMats = (uint32_t)meshMats.size();
        m_MaterialsTable = MaterialTable::Create(numMats);
        for (uint32_t i = 0; i < numMats; i++)
            m_MaterialsTable->SetMaterial(i, MaterialAsset::Create(meshMats[i]));
    }

    StaticMesh::StaticMesh(const Ref<StaticMesh>& other)
        : m_MeshSource(other->m_MeshSource), m_MaterialsTable(other->m_MaterialsTable)
    {
        SetSubMeshes(other->m_SubMeshes);
    }

    StaticMesh::~StaticMesh()
    {
    }

    void StaticMesh::SetSubMeshes(const std::vector<uint32_t>& subMeshes)
    {
        if (!subMeshes.empty())
        {
            m_SubMeshes = subMeshes;
        }
        else
        {
            const std::vector<SubMesh>& subMeshs = m_MeshSource->GetSubMeshes();
            m_SubMeshes.resize(subMeshs.size());
            for (uint32_t i = 0; i < subMeshs.size(); i++)
                m_SubMeshes[i] = i;
        }
    }

}