#include "Aurorapch.h"
#include "Material.h"

#include "Renderer/Renderer.h"

namespace Aurora {	

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////// Material
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	Ref<Material> Material::Create(const std::string& name, const Ref<Shader>& shader)
	{
		return CreateRef<Material>(name, shader);
	}

	Material::Material(const std::string& name, const Ref<Shader>& shader)
		: m_Shader(shader)
	{
		AllocateStorage();

		m_MaterialFlags |= (uint32_t)MaterialFlag::DepthTest;
		m_MaterialFlags |= (uint32_t)MaterialFlag::Blend;
	}

	Material::~Material()
	{
		m_UniformStorageBuffer.Release();
	}

	void Material::SetUpForRendering() const
	{
		m_Shader->Bind();

		const auto& shaderBuffers = m_Shader->GetShaderBuffers();
		// Here we are checking if <= 2 since it is possible to have one push_constant in the vertex shader and another in
		// the fragment shader and so this is considered pushc_constant buffer sharing
		AR_CORE_ASSERT(shaderBuffers.size() <= 2, "No more than 2 push_constant buffers are allowed per program!");

		for (const auto& [bufferName, buffer] : shaderBuffers)
		{
			// This will run once since each shader will contain at most 1 push_constant buffer
			// however keep it a loop in case vulkan ever think about removing that restriction
			for (const auto& [name, uniform] : buffer.Uniforms)
			{
				switch (uniform.GetUniformType())
				{
					//  TODO: I dont think the bool case is handled correctly with the uint32_t...!
					case ShaderUniformType::Bool:
					{
						const uint32_t value = m_UniformStorageBuffer.Read<uint32_t>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::UInt:
					{
						const uint32_t value = m_UniformStorageBuffer.Read<uint32_t>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::Int:
					{
						const int value = m_UniformStorageBuffer.Read<int>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::IVec2:
					{
						const glm::ivec2& value = m_UniformStorageBuffer.Read<glm::ivec2>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::IVec3:
					{
						const glm::ivec3& value = m_UniformStorageBuffer.Read<glm::ivec3>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::IVec4:
					{
						const glm::ivec4& value = m_UniformStorageBuffer.Read<glm::ivec4>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::Float:
					{
						const float value = m_UniformStorageBuffer.Read<float>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::Vec2:
					{
						const glm::vec2& value = m_UniformStorageBuffer.Read<glm::vec2>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::Vec3:
					{
						const glm::vec3& value = m_UniformStorageBuffer.Read<glm::vec3>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::Vec4:
					{
						const glm::vec4& value = m_UniformStorageBuffer.Read<glm::vec4>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::Mat3:
					{
						const glm::mat3& value = m_UniformStorageBuffer.Read<glm::mat3>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					case ShaderUniformType::Mat4:
					{
						const glm::mat4& value = m_UniformStorageBuffer.Read<glm::mat4>(uniform.GetOffset());
						m_Shader->SetUniform(name, value);
						break;
					}
					default:
					{
						AR_CORE_ASSERT(false);
						break;
					}
				}
			}
		}

		for (const auto& [slot, texture] : m_Texture2Ds)
		{
			texture->Bind(slot);
		}

		for (const auto& [slot, texture] : m_CubeTextures)
		{
			texture->Bind(slot);
		}
	}

	void Material::AllocateStorage()
	{
		const auto& shaderBuffers = m_Shader->GetShaderBuffers();

		if (shaderBuffers.size())
		{
			uint32_t size = 0;
			for (const auto& [name, buffer] : shaderBuffers)
				size += buffer.Size;

			m_UniformStorageBuffer.Allocate(size);
		}
	}

	void Material::Set(std::string_view fullname, float value) const
	{
		Set<float>(fullname, value);
	}

	void Material::Set(std::string_view fullname, int value) const
	{
		Set<int>(fullname, value);
	}

	void Material::Set(std::string_view fullname, uint32_t value) const
	{
		Set<uint32_t>(fullname, value);
	}

	void Material::Set(std::string_view fullname, bool value) const
	{
		Set<bool>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const glm::ivec2& value) const
	{
		Set<glm::ivec2>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const glm::ivec3& value) const
	{
		Set<glm::ivec3>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const glm::ivec4& value) const
	{
		Set<glm::ivec4>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const glm::vec2& value) const
	{
		Set<glm::vec2>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const glm::vec3& value) const
	{
		Set<glm::vec3>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const glm::vec4& value) const
	{
		Set<glm::vec4>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const glm::mat3& value) const
	{
		Set<glm::mat3>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const glm::mat4& value) const
	{
		Set<glm::mat4>(fullname, value);
	}

	void Material::Set(std::string_view fullname, const Ref<Texture2D>& texture) const
	{
		const ShaderResourceDeclaration* decl = FindResourceDeclaration(fullname);
		if (!decl)
		{
			AR_CORE_WARN_TAG("Material", "Cannot find material property: {0}", fullname);
			return;
		}

		uint32_t slot = decl->GetRegister();
		m_Texture2Ds[slot] = texture;
	}

	void Material::Set(std::string_view fullname, const Ref<CubeTexture>& cubeTexture) const
	{
		const ShaderResourceDeclaration* decl = FindResourceDeclaration(fullname);
		if (!decl)
		{
			AR_CORE_WARN_TAG("Material", "Cannot find material property: {0}", fullname);
			return;
		}

		uint32_t slot = decl->GetRegister();
		m_CubeTextures[slot] = cubeTexture;
	}

	float& Material::GetFloat(std::string_view name) const
	{
		return Get<float>(name);
	}

	int32_t& Material::GetInt(std::string_view name) const
	{
		return Get<int32_t>(name);
	}

	uint32_t& Material::GetUInt(std::string_view name) const
	{
		return Get<uint32_t>(name);
	}

	bool& Material::GetBool(std::string_view name) const
	{
		return Get<bool>(name);
	}

	glm::ivec2& Material::GetIntVec2(std::string_view name) const
	{
		return Get<glm::ivec2>(name);
	}

	glm::ivec3& Material::GetIntVec3(std::string_view name) const
	{
		return Get<glm::ivec3>(name);
	}

	glm::ivec4& Material::GetIntVec4(std::string_view name) const
	{
		return Get<glm::ivec4>(name);
	}

	glm::vec2& Material::GetVec2(std::string_view name) const
	{
		return Get<glm::vec2>(name);
	}

	glm::vec3& Material::GetVec3(std::string_view name) const
	{
		return Get<glm::vec3>(name);
	}

	glm::vec4& Material::GetVec4(std::string_view name) const
	{
		return Get<glm::vec4>(name);
	}

	glm::mat3& Material::GetMat3(std::string_view name) const
	{
		return Get<glm::mat3>(name);
	}

	glm::mat4& Material::GetMat4(std::string_view name) const
	{
		return Get<glm::mat4>(name);
	}

	Ref<Texture2D> Material::GetTexture2D(std::string_view name)
	{
		const ShaderResourceDeclaration* decl = FindResourceDeclaration(name);
		AR_CORE_ASSERT(decl, "Could not find resource!");

		uint32_t slot = decl->GetRegister();
		AR_CORE_ASSERT(slot <= m_Texture2Ds.size(), "Texture slot is invalid!"); // TODO: This may not be <= rather just <..!
		return m_Texture2Ds[slot];
	}

	Ref<CubeTexture> Material::GetCubeTexture(const std::string& name)
	{
		const ShaderResourceDeclaration* decl = FindResourceDeclaration(name);
		AR_CORE_ASSERT(decl, "Could not find resource!");

		uint32_t slot = decl->GetRegister();
		AR_CORE_ASSERT(slot <= m_CubeTextures.size(), "Texture slot is invalid!"); // TODO: This may not be <= rather just <..!
		return m_CubeTextures[slot];
	}

	Ref<Texture2D> Material::TryGetTexture2D(std::string_view name)
	{
		const ShaderResourceDeclaration* decl = FindResourceDeclaration(name);
		if (!decl)
			return nullptr;

		uint32_t slot = decl->GetRegister();
		if (m_Texture2Ds.find(slot) == m_Texture2Ds.end())
			return nullptr;

		return m_Texture2Ds[slot];
	}

	Ref<CubeTexture> Material::TryGetCubeTexture(std::string_view name)
	{
		const ShaderResourceDeclaration* decl = FindResourceDeclaration(name);
		if (!decl)
			return nullptr;

		uint32_t slot = decl->GetRegister();
		if (m_CubeTextures.find(slot) == m_CubeTextures.end())
			return nullptr;

		return m_CubeTextures[slot];
	}

	void Material::SetFlag(MaterialFlag flag, bool value)
	{
		if (value)
		{
			m_MaterialFlags |= (uint32_t)flag;
		}
		else
		{
			m_MaterialFlags &= ~(uint32_t)flag;
		}
	}

	const ShaderUniform* Material::FindUniformDeclaration(std::string_view name) const
	{
		std::string nameStr = std::string(name);
		const auto& shaderBuffers = m_Shader->GetShaderBuffers();

		// Here we are checking if <= 2 since it is possible to have one push_constant in the vertex shader and another in
		// the fragment shader and so this is considered pushc_constant buffer sharing
		AR_CORE_ASSERT(shaderBuffers.size() <= 2, "No more than 2 push_constant buffers are allowed per program!");

		for (const auto& [bufferName, buffer] : shaderBuffers)
		{
			if (buffer.Uniforms.find(nameStr) != buffer.Uniforms.end())
				return &(buffer.Uniforms.at(nameStr));
		}

		return nullptr;
	}

	const ShaderResourceDeclaration* Material::FindResourceDeclaration(std::string_view name) const
	{
		const auto& resources = m_Shader->GetShaderResources();

		for (const auto& [resName, res] : resources)
		{
			if (res.GetName() == name)
				return &res;
		}

		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////// MaterialAsset
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	static const std::string s_AlbedoColor = "u_MaterialUniforms.AlbedoColor";
	static const std::string s_UseNormalMap = "u_MaterialUniforms.UseNormalMap";
	static const std::string s_Roughness = "u_MaterialUniforms.Roughness";
	static const std::string s_Metalness = "u_MaterialUniforms.Metalness";

	// Textures inputs...
	static const std::string s_AlbedoMap = "u_AlbedoTexture";
	static const std::string s_NormalMap = "u_NormalTexture";
	static const std::string s_RoughnessMap = "u_RoughnessTexture";
	static const std::string s_MetalnessMap = "u_MetalnessTexture";

	Ref<MaterialAsset> Create()
	{
		return CreateRef<MaterialAsset>();
	}

	Ref<MaterialAsset> Create(Ref<Material> material)
	{
		return CreateRef<MaterialAsset>(material);
	}

	MaterialAsset::MaterialAsset()
	{
		// Default name is to set in the SceneHierarchyPanel
		m_Material = Material::Create("", Renderer::GetShaderLibrary()->Get("AuroraPBRStatic"));

		ResetToDefault();
	}

	MaterialAsset::MaterialAsset(Ref<Material> material)
	{
		m_Material = Material::Create(material->GetName(), material->GetShader());
	}

	MaterialAsset::~MaterialAsset()
	{
	}

	void MaterialAsset::SetAlbedoColor(const glm::vec3& color)
	{
		m_Material->Set(s_AlbedoColor, color);
	}

	glm::vec3& MaterialAsset::GetAlbedoColor()
	{
		return m_Material->GetVec3(s_AlbedoColor);
	}

	void MaterialAsset::SetRoughness(float roughness)
	{
		m_Material->Set(s_Roughness, roughness);
	}

	float& MaterialAsset::GetRoughness()
	{
		return m_Material->GetFloat(s_Roughness);
	}

	void MaterialAsset::SetMetalness(float metalness)
	{
		m_Material->Set(s_Metalness, metalness);
	}

	float& MaterialAsset::GetMetalness()
	{
		return m_Material->GetFloat(s_Metalness);
	}

	void MaterialAsset::SetAlbedoMap(Ref<Texture2D> albedoMap)
	{
		m_Material->Set(s_AlbedoMap, albedoMap);
	}

	void MaterialAsset::ClearAlbedoMap()
	{
		m_Material->Set(s_AlbedoMap, Renderer::GetWhiteTexture());
	}

	Ref<Texture2D> MaterialAsset::GetAlbedoMap()
	{
		return m_Material->TryGetTexture2D(s_AlbedoMap);
	}

	void MaterialAsset::SetNormalMap(Ref<Texture2D> normalMap)
	{
		m_Material->Set(s_NormalMap, normalMap);
	}

	void MaterialAsset::ClearNormalMap()
	{
		m_Material->Set(s_NormalMap, Renderer::GetWhiteTexture());
	}

	void MaterialAsset::UseNormalMap(bool use)
	{
		m_Material->Set(s_UseNormalMap, use);
	}

	Ref<Texture2D> MaterialAsset::GetNormalMap()
	{
		return m_Material->TryGetTexture2D(s_NormalMap);
	}

	bool MaterialAsset::IsUsingNormalMap()
	{
		return m_Material->GetBool(s_UseNormalMap);
	}

	void MaterialAsset::SetRoughnessMap(Ref<Texture2D> roughnessMap)
	{
		m_Material->Set(s_RoughnessMap, roughnessMap);
	}

	void MaterialAsset::ClearRoughnessMap()
	{
		m_Material->Set(s_RoughnessMap, Renderer::GetWhiteTexture());
	}

	Ref<Texture2D> MaterialAsset::GetRoughnessMap()
	{
		return m_Material->TryGetTexture2D(s_RoughnessMap);
	}

	void MaterialAsset::SetMetalnessMap(Ref<Texture2D> metalnessMap)
	{
		m_Material->Set(s_MetalnessMap, metalnessMap);
	}

	void MaterialAsset::ClearMetalnessMap()
	{
		m_Material->Set(s_MetalnessMap, Renderer::GetWhiteTexture());
	}

	Ref<Texture2D> MaterialAsset::GetMetalnessMap()
	{
		return m_Material->TryGetTexture2D(s_MetalnessMap);
	}

	void MaterialAsset::ResetToDefault()
	{
		SetAlbedoColor({ 0.8f, 0.8f, 0.8f });
		UseNormalMap(false);
		SetRoughness(0.4f);
		SetMetalness(0.0f);

		SetAlbedoMap(Renderer::GetWhiteTexture());
		SetNormalMap(Renderer::GetWhiteTexture());
		SetRoughnessMap(Renderer::GetWhiteTexture());
		SetMetalnessMap(Renderer::GetWhiteTexture());
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////// MaterialTable
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	Ref<MaterialTable> Create(uint32_t count = 1)
	{
		return CreateRef<MaterialTable>(count);
	}

	Ref<MaterialTable> Create(Ref<MaterialTable> other)
	{
		return CreateRef<MaterialTable>(other);
	}

	MaterialTable::MaterialTable(uint32_t count)
		: m_MaterialCount(count)
	{
	}

	MaterialTable::MaterialTable(Ref<MaterialTable> other)
		: m_MaterialCount(other->m_MaterialCount)
	{
		const std::map<uint32_t, Ref<MaterialAsset>>& meshMaterials = other->GetMaterials();
		for (const auto& [index, materialAsset] : meshMaterials)
			SetMaterial(index, materialAsset);
	}

	MaterialTable::~MaterialTable()
	{
	}

	void MaterialTable::SetMaterial(uint32_t index, Ref<MaterialAsset> material)
	{
		m_Materials[index] = material;
		if (index >= m_MaterialCount)
			m_MaterialCount = index + 1;
	}

	void MaterialTable::ClearMaterial(uint32_t index)
	{
		AR_CORE_ASSERT(HasMaterial(index));
		m_Materials.erase(index);
		if (index >= m_MaterialCount)
			m_MaterialCount = index + 1;
	}

	void MaterialTable::Clear()
	{
		m_Materials.clear();
	}

}