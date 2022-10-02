#pragma once

#include "Core/Base.h"
#include "Core/AABB.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Material.h"

#include <glm/glm.hpp>
#include <vector>

/*
 * Taken straight from Hazel: https://hazelengine.com/ since i know nothing on how to use assimp. It was a good
 * learning experience
 */

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp {
    class Importer;
}

namespace Aurora {

    struct Vertex 
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Binormal;
        glm::vec2 TexCoords;
    };

    static const int s_NumAttributes = 5;
    
    struct Index
    {
        uint32_t V1;
        uint32_t V2;
        uint32_t V3;
    };

    static_assert(sizeof(Index) == 3 * sizeof(uint32_t));

    struct Triangle
    {
        Vertex V1;
        Vertex V2;
        Vertex V3;

        Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
            : V1(v1), V2(v2), V3(v3) {}
    };

    struct SubMesh
    {
        uint32_t BaseVertex;
        uint32_t BaseIndex;
        uint32_t MaterialIndex;
        uint32_t IndexCount;
        uint32_t VertexCount;

        glm::mat4 Transform = glm::mat4(1.0f); // World transform
        glm::mat4 LocalTransform = glm::mat4(1.0f);
        AABB BoundingBox;

        std::string NodeName;
        std::string MeshName;
        bool IsRigged = false;
    };

    class MeshSource : public RefCountedObject
    {
    public:
        MeshSource(const std::filesystem::path& filePath);
        MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform);
        MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const std::vector<SubMesh>& subMeshes);
        virtual ~MeshSource();

        static Ref<MeshSource> Create(const std::filesystem::path filePath);
        static Ref<MeshSource> Create(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform);
        static Ref<MeshSource> Create(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const std::vector<SubMesh>& subMeshes);

        void DumpVertexBuffer();

        [[nodiscard]] std::vector<SubMesh>& GetSubMeshes() { return m_SubMeshes; }
        [[nodiscard]] const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }

        [[nodiscard]] const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
        [[nodiscard]] const std::vector<Index>& GetIndices() const { return m_Indices; }

        [[nodiscard]] bool IsSubmeshRigged(uint32_t submeshIndex) const { return m_SubMeshes[submeshIndex].IsRigged; }

        [[nodiscard]] std::vector<Ref<Material>>& GetMaterials() { return m_Materials; }
        [[nodiscard]] const std::vector<Ref<Material>>& GetMaterials() const { return m_Materials; }
        [[nodiscard]] const std::filesystem::path& GetAssetPath() const { return m_AssetPath; }

        [[nodiscard]] const std::vector<Triangle> GetTriangleCache(uint32_t index) const { return m_TriangleCache.at(index); }

        [[nodiscard]] Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
        [[nodiscard]] Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }
        [[nodiscard]] Ref<VertexArray> GetVertexArray() { return m_VertexArray; }

        [[nodiscard]] const AABB& GetBoundingBox() const { return m_BoundingBox; }

    private:
        void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);

    private:
        std::vector<SubMesh> m_SubMeshes;

        // note: the importer owns data pointed to by m_Scene, and m_NodeMap and hence must stay in scope for lifetime of MeshSource.
        Scope<Assimp::Importer> m_Importer;

        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;
        // For OpenGL this is really all i need to draw the model i can just bind this and is index buffer to draw
        Ref<VertexArray> m_VertexArray;

        std::vector<Vertex> m_Vertices;
        std::vector<Index> m_Indices;
        std::unordered_map<aiNode*, std::vector<uint32_t>> m_NodeMap;
        const aiScene* m_Scene;

        std::vector<Ref<Material>> m_Materials;

        std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;

        AABB m_BoundingBox;

        std::filesystem::path m_AssetPath;

        friend class Scene;
        friend class Renderer;
        // TODO: Should be SceneHierarchyPanel and MeshViewerPanel once these are somethings
        friend class EditorLayer;
        friend class Mesh;

    };

    class StaticMesh : public RefCountedObject
    {
    public:
        StaticMesh(Ref<MeshSource> meshSource);
        StaticMesh(Ref<MeshSource> meshSource, const std::vector<uint32_t>& subMeshes);
        StaticMesh(const Ref<StaticMesh>& other);
        virtual ~StaticMesh();

        static Ref<StaticMesh> Create(Ref<MeshSource> meshSource);
        static Ref<StaticMesh> Create(Ref<MeshSource> meshSource, const std::vector<uint32_t>& subMeshes);

        // Pass in an empty vector to set all submeshes of meshSource!
        void SetSubMeshes(const std::vector<uint32_t>& subMeshes);

        [[nodiscard]] std::vector<uint32_t>& GetSubMeshes() { return m_SubMeshes; }
        [[nodiscard]] const std::vector<uint32_t>& GetSubMeshes() const { return m_SubMeshes; }

        void SetMeshSource(Ref<MeshSource> meshSource) { m_MeshSource = meshSource; }

        [[nodiscard]] Ref<MeshSource> GetMeshSource() { return m_MeshSource; }
        [[nodiscard]] Ref<MeshSource> GetMeshSource() const { return m_MeshSource; }

        [[nodiscard]] const Ref<MaterialTable>& GetMaterials() const { return m_MaterialsTable; }

    private:
        Ref<MeshSource> m_MeshSource;
        std::vector<uint32_t> m_SubMeshes;

        Ref<MaterialTable> m_MaterialsTable;

        friend class Scene;
        friend class Renderer;
        // TODO: Should be SceneHierarchyPanel and MeshViewerPanel once these are a thing
        friend class EditorLayer;

    };

}