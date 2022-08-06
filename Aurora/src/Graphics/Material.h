#pragma once

// NOT IMPLEMENTED!!

#include "Core/Base.h"
#include "Shader.h"
#include "Texture.h"

namespace Aurora {

	// TODO: Render Flags...?
	class Material
	{
	public:
		Material(const Ref<Shader>& shader);
		~Material();

		void Bind();
		void UnBind();

		// Looks like this setuniform is temp
		void SetUniform(const std::string& name, uint32_t val);
		void SetTexture(const std::string& name, const Ref<Texture>& texture);

		void SetDiffuse(const glm::vec4& color);
		void SetSpecular(const glm::vec4& color);
		void SetShininess(float shiny);

		void SetAttenuationFactors(float constant, float linear, float quadratic);

		void SetDiffuseMap(const Ref<Texture>& texture);
		void SetSpecularMap(const Ref<Texture>& texture);

	private:
		Ref<Shader> m_Shader;
		std::vector<Ref<Texture>> m_Textures;

	};

}