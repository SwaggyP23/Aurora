#include "Aurorapch.h"
#include "Material.h"

// NOT IMPLEMENTED!!

namespace Aurora {

	Material::Material(const Ref<Shader>& shader)
		: m_Shader(shader)
	{
	}

	Material::~Material()
	{
	}

	void Material::Bind()
	{
		m_Shader->Bind();

		for (int i = 0; i < m_Textures.size(); i++)
			if(m_Textures[i])
				m_Textures[i]->Bind(i);
	}

	void Material::UnBind() // I dont think this is necessary to unbind all the textures neither the shader
	{
		m_Shader->UnBind();

		for (int i = 0; i < m_Textures.size(); i++)
			if(m_Textures[i])
				m_Textures[i]->UnBind(i);
	}

	void Material::SetUniform(const std::string& name, uint32_t val)
	{
		m_Shader->SetUniform1i(name.c_str(), val);
	}

	void Material::SetTexture(const std::string& name, const Ref<Texture>& texture)
	{
		m_Textures.push_back(texture);
	}

	void Material::SetDiffuse(const glm::vec4& color)
	{
		m_Shader->SetUniform4f("u_DiffColor", color);
	}

	void Material::SetSpecular(const glm::vec4& color)
	{
		m_Shader->SetUniform4f("u_SpecColor", color);
	}

	void Material::SetShininess(float shiny)
	{
		m_Shader->SetUniform1f("u_Shininess", shiny);
	}

	void Material::SetAttenuationFactors(float constant, float linear, float quadratic)
	{
		m_Shader->SetUniform1f("u_Constant", constant);
		m_Shader->SetUniform1f("u_Linear", linear);
		m_Shader->SetUniform1f("u_Quadratic", quadratic);
	}

	void Material::SetDiffuseMap(const Ref<Texture>& texture)
	{
		SetTexture("u_DiffMap", texture);
		m_Textures.push_back(texture);
	}

	void Material::SetSpecularMap(const Ref<Texture>& texture)
	{
		SetTexture("u_SpecMap", texture);
		m_Textures.push_back(texture);
	}

}