#pragma once

#include "Core/Base.h"
#include "Core/Buffer.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <map>

/*
 * Currently all materials in Aurora will be PBR materials!
 * 
 * Still needs some looking and checking since i am not so sure of the architecture i made, and may(probably will)
 * rework most of the stuff
 * 
 * TODO: Actually use the material flags to get different rendering bahviour
 */

namespace Aurora {

	enum class MaterialFlag : uint8_t
	{
		None           = BIT(0),
		DepthTest      = BIT(1),
		Blend          = BIT(2),
		TwoSided       = BIT(3)
	};

	class Material : public RefCountedObject
	{
	public:
		Material(const std::string& name, const Ref<Shader>& shader);
		virtual ~Material();

		static Ref<Material> Create(const std::string& name, const Ref<Shader>& shader);

		void SetUpForRendering() const;

		// Uniform Setters..
		void Set(std::string_view fullname, float value) const;
		void Set(std::string_view fullname, int value) const;
		void Set(std::string_view fullname, uint32_t value) const;
		void Set(std::string_view fullname, bool value) const;

		void Set(std::string_view fullname, const glm::ivec2& value) const;
		void Set(std::string_view fullname, const glm::ivec3& value) const;
		void Set(std::string_view fullname, const glm::ivec4& value) const;

		void Set(std::string_view fullname, const glm::vec2& value) const;
		void Set(std::string_view fullname, const glm::vec3& value) const;
		void Set(std::string_view fullname, const glm::vec4& value) const;
		void Set(std::string_view fullname, const glm::mat3& value) const;
		void Set(std::string_view fullname, const glm::mat4& value) const;

		// Texture Setters.. Not sure if these should be constant
		void Set(std::string_view fullname, const Ref<Texture2D>& texture) const;
		void Set(std::string_view, const Ref<CubeTexture>& cubeTexture) const;
		//void Set(const std::string& fullname, const Ref<Image2D>& image); // Currently not in use untill image2d is actually made

		// Uniform Getters..
		[[nodiscard]] bool& GetBool(std::string_view name) const;
		[[nodiscard]] int32_t& GetInt(std::string_view name) const;
		[[nodiscard]] uint32_t& GetUInt(std::string_view name) const;
		[[nodiscard]] float& GetFloat(std::string_view name) const;

		[[nodiscard]] glm::ivec2& GetIntVec2(std::string_view name) const;
		[[nodiscard]] glm::ivec3& GetIntVec3(std::string_view name) const;
		[[nodiscard]] glm::ivec4& GetIntVec4(std::string_view name) const;

		[[nodiscard]] glm::vec2& GetVec2(std::string_view name) const;
		[[nodiscard]] glm::vec3& GetVec3(std::string_view name) const;
		[[nodiscard]] glm::vec4& GetVec4(std::string_view name) const;
		[[nodiscard]] glm::mat3& GetMat3(std::string_view name) const;
		[[nodiscard]] glm::mat4& GetMat4(std::string_view name) const;

		// Texture Getters..
		[[nodiscard]] Ref<Texture2D> GetTexture2D(std::string_view name);
		[[nodiscard]] Ref<CubeTexture> GetCubeTexture(const std::string& name);

		[[nodiscard]] Ref<Texture2D> TryGetTexture2D(std::string_view name);
		[[nodiscard]] Ref<CubeTexture> TryGetCubeTexture(std::string_view name);

		[[nodiscard]] uint32_t GetFlags() const { return m_MaterialFlags; }
		[[nodiscard]] bool HasFlag(MaterialFlag flag) const { return (uint32_t)flag & (uint32_t)m_MaterialFlags; }
		void SetFlag(MaterialFlag flag, bool value);

		[[nodiscard]] Ref<Shader> GetShader() const { return m_Shader; }
		[[nodiscard]] const std::string& GetName() const { return m_Name; }
		[[nodiscard]] const Buffer& GetUniformBufferStorage() const { return m_UniformStorageBuffer; }

	private:
		void AllocateStorage();

		template<typename T>
		void Set(std::string_view fullname, const T& value) const
		{
			const ShaderUniform* declaration = FindUniformDeclaration(fullname);
			if (!declaration)
			{
				AR_CORE_WARN_TAG("Material", "Cannot find uniform with name: {0}", fullname);
				return;
			}

			Buffer& buffer = m_UniformStorageBuffer;
			buffer.Write((Byte*)&value, declaration->GetSize(), declaration->GetOffset());
		}

		template<typename T>
		T* Get(std::string_view fullname) const
		{
			const ShaderUniform* declaration = FindUniformDeclaration(fullname);
			if (!declaration)
			{
				// If this prints then probs are u messed up matching the uniform names from code with shader or wrong shader / wrong uniform
				AR_CORE_WARN_TAG("Material", "Could not find the uniform with name: {0}", fullname);
				return nullptr;
			}

			Buffer& buffer = m_UniformStorageBuffer;
			return &(buffer.Read<T>(declaration->GetOffset()));
		}

		const ShaderUniform* FindUniformDeclaration(std::string_view name) const;
		const ShaderResourceDeclaration* FindResourceDeclaration(std::string_view name) const;

	private:
		std::string m_Name;
		Ref<Shader> m_Shader;

		// TODO: These flags are currently ignored when rendering...
		uint32_t m_MaterialFlags = 0;

		mutable Buffer m_UniformStorageBuffer;
		// These are automatically sorted according to their slot index which is perfect!
		mutable std::map<uint32_t, Ref<Texture2D>> m_Texture2Ds;
		mutable std::map<uint32_t, Ref<CubeTexture>> m_CubeTextures;

	};

	class MaterialAsset : public RefCountedObject
	{
	public:
		MaterialAsset();
		MaterialAsset(Ref<Material> material);
		virtual ~MaterialAsset();

		static Ref<MaterialAsset> Create();
		static Ref<MaterialAsset> Create(Ref<Material> material);

		void SetAlbedoColor(const glm::vec3& color);
		[[nodiscard]] glm::vec3& GetAlbedoColor();

		void SetRoughness(float roughness);
		[[nodiscard]] float& GetRoughness();

		void SetMetalness(float metalness);
		[[nodiscard]] float& GetMetalness();

		void SetAlbedoMap(Ref<Texture2D> albedoMap);
		void ClearAlbedoMap();
		[[nodiscard]] Ref<Texture2D> GetAlbedoMap();

		void SetNormalMap(Ref<Texture2D> normalMap);
		void ClearNormalMap();
		void UseNormalMap(bool use);
		[[nodiscard]] Ref<Texture2D> GetNormalMap();
		[[nodiscard]] bool IsUsingNormalMap();

		void SetRoughnessMap(Ref<Texture2D> roughnessMap);
		void ClearRoughnessMap();
		[[nodiscard]] Ref<Texture2D> GetRoughnessMap();

		void SetMetalnessMap(Ref<Texture2D> metalnessMap);
		void ClearMetalnessMap();
		[[nodiscard]] Ref<Texture2D> GetMetalnessMap();

		void SetMaterial(Ref<Material> material) { m_Material = material; }
		[[nodiscard]] Ref<Material> GetMaterial() const { return m_Material; }

	private:
		void ResetToDefault();

	private:
		Ref<Material> m_Material;

		// TODO: Sould be MaterialEditor once that is a thing
		friend class EditorLayer;

	};

	// Just a wrapper over a map of indices to material assets
	class MaterialTable : public RefCountedObject
	{
	public:
		MaterialTable(uint32_t count = 1);
		MaterialTable(Ref<MaterialTable> other);
		virtual ~MaterialTable();

		static Ref<MaterialTable> Create(uint32_t count = 1);
		static Ref<MaterialTable> Create(Ref<MaterialTable> other);

		[[nodiscard]] bool HasMaterial(uint32_t materialIndex) const { return m_Materials.find(materialIndex) != m_Materials.end(); }

		void SetMaterial(uint32_t index, Ref<MaterialAsset> material);
		void ClearMaterial(uint32_t index);

		[[nodiscard]] Ref<MaterialAsset> GetMaterial(uint32_t materialIndex) const { AR_CORE_ASSERT(HasMaterial(materialIndex)); return m_Materials.at(materialIndex); }

		[[nodiscard]] std::map<uint32_t, Ref<MaterialAsset>>& GetMaterials() { return m_Materials; }
		[[nodiscard]] const std::map<uint32_t, Ref<MaterialAsset>>& GetMaterials() const { return m_Materials; }
		[[nodiscard]] uint32_t GetMaterialCount() const { return m_MaterialCount; }
		void SetMaterialCount(uint32_t count) { m_MaterialCount = count; }

		void Clear();

	private:
		std::map<uint32_t, Ref<MaterialAsset>> m_Materials;
		uint32_t m_MaterialCount = 0; // Number of held materials/materials to hold
	};

}