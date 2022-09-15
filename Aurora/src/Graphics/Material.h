#pragma once

// NOT IMPLEMENTED!!

#include "Core/Base.h"
#include "Core/Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "CubeTexture.h"
#include "UniformBuffer.h"

#include <glm/glm.hpp>

/*
 * Currently all materials in Aurora will be PBR materials!
 * 
 * Still needs some looking and checking since i am not so sure of the architecture i made, and may(probably will) rework most
 * of the stuff!
 * 
 * TODO: Check and rework most of the necessary stuff!
 */

namespace Aurora {

	enum class MaterialFlag
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
		~Material();

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
		float& GetFloat(std::string_view name) const;
		int32_t& GetInt(std::string_view name) const;
		uint32_t& GetUInt(std::string_view name) const;
		bool& GetBool(std::string_view name) const;

		glm::ivec2& GetIntVec2(std::string_view name) const;
		glm::ivec3& GetIntVec3(std::string_view name) const;
		glm::ivec4& GetIntVec4(std::string_view name) const;

		glm::vec2& GetVec2(std::string_view name) const;
		glm::vec3& GetVec3(std::string_view name) const;
		glm::vec4& GetVec4(std::string_view name) const;
		glm::mat3& GetMat3(std::string_view name) const;
		glm::mat4& GetMat4(std::string_view name) const;

		// Texture Getters..
		Ref<Texture2D> GetTexture2D(std::string_view name);
		Ref<CubeTexture> GetCubeTexture(const std::string& name);
		//Ref<Image2D> GetImage2D(const std::string& name) const; // TODO: Currently not in use untill image2d is actually made

		Ref<Texture2D> TryGetTexture2D(std::string_view name);

		uint32_t GetFlags() const { return m_MaterialFlags; }
		bool HasFlag(MaterialFlag flag) const { return (uint32_t)flag & m_MaterialFlags; }
		void SetFlag(MaterialFlag flag, bool value);

		const Ref<Shader>& GetShader() const { return m_Shader; }
		const std::string& GetName() const { return m_Name; }

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
		T& Get(std::string_view fullname) const
		{
			const ShaderUniform* declaration = FindUniformDeclaration(fullname);
			if (!declaration)
			{
				AR_CORE_WARN_TAG("Material", "Could not find the uniform with name: {0}", fullname);
				T nullvalue = T();
				return nullvalue;
			}

			Buffer& buffer = m_UniformStorageBuffer;
			return buffer.Read<T>(declaration->GetOffset());
		}

		// TODO: This currently does not work...!
		template<typename T>
		Ref<T> TryGetResource(const std::string& name)
		{
			auto decl = FindResourceDeclaration(name);
			if (!decl)
				return nullptr;

			uint32_t slot = decl->GetRegister();
			if (slot >= m_Textures.size())
				return nullptr;

			return Ref<T>((const Ref<Texture>&)m_Textures[slot]);
		}

		const ShaderUniform* FindUniformDeclaration(std::string_view name) const;
		const ShaderResourceDeclaration* FindResourceDeclaration(std::string_view name) const;

	private:
		std::string m_Name;

		Ref<Shader> m_Shader;
		uint32_t m_MaterialFlags = 0;

		mutable Buffer m_UniformStorageBuffer;
		// These are automatically sorted according to their slot index which is perfect!
		mutable std::map<uint32_t, Ref<Texture2D>> m_Texture2Ds;
		mutable std::map<uint32_t, Ref<CubeTexture>> m_CubeTextures;

	};

}