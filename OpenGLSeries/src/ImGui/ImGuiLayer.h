#pragma once

/*
 * The ImGuiLayer and Layers and Events system is all thanks to The Cherno which infact this whole thing 
 * is in thanks to him since he is the main reference and then there is also the LearnOpenGL Book
 */

#include "Layers/Layer.h"
#include "Events/ApplicationEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"
#include <glm/glm.hpp>

class ImGuiLayer : public Layer
{
public:
	ImGuiLayer();
	~ImGuiLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onImGuiRender() override;

	void begin();
	void end();

	inline float getBlend() const { return m_Blend; }
	inline float getAmbLight() const { return m_AmbLight; }
	inline float getRotation() const { return m_Rotation; }

	inline const glm::vec3& getTransalations() const { return m_Transalations; }
	inline const glm::vec3& getScales() const { return m_Scale; }

	inline const glm::vec3& getLightScales() const { return m_LightScale; }
	inline const glm::vec3& getLightTranslations() const { return m_LightTransalations; }

	inline const glm::vec3& getGroundScales() const { return m_GroundScale; }
	inline const glm::vec3& getGroundTranslations() const { return m_GroundTransalations; }

	inline const glm::vec3& getSphereScales() const { return m_SphereScale; }
	inline const glm::vec3& getSphereTranslations() const { return m_SphereTransalations; }

	inline const glm::vec4& getUniColor() const { return m_UniColor; }
	inline const glm::vec4& getLightColor() const { return m_LightColor; }
	inline const glm::vec4& getClearColor() const { return m_Color; }

private:
	float m_Time = 0.0f;
	float m_Blend = 0.0f;
	float m_AmbLight = 0.06f;
	float m_Rotation = 0.0f;

	glm::vec3 m_Transalations = glm::vec3(0.0f);
	glm::vec3 m_Scale = glm::vec3(1.0f);

	glm::vec3 m_LightTransalations = glm::vec3(2.0f);
	glm::vec3 m_LightScale = glm::vec3(0.2f);

	glm::vec3 m_GroundTransalations = glm::vec3(0.0f);
	glm::vec3 m_GroundScale = glm::vec3(1.0f);

	glm::vec3 m_SphereTransalations = glm::vec3(0.0f);
	glm::vec3 m_SphereScale = glm::vec3(1.0f);

	//glm::vec4 m_Color = glm::vec4(0.2f, 0.6f, 0.6f, 1.0f); // Initial clear color.
	glm::vec4 m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Initial clear color for lighting.
	glm::vec4 m_LightColor = glm::vec4(1.0f); // Initial clear color for lighting.
	glm::vec4 m_UniColor = glm::vec4(1.0f);

};