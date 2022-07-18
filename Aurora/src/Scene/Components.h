#pragma once

// TODO: to be added, mesh components by refering to darianopolis on discord

#include <glm/glm.hpp>

namespace Aurora {

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(std::string tag)
			: Tag(tag) {}
		TagComponent(const TagComponent&) = default;

	};

	struct TransformComponent
	{
		glm::vec3 Translation{ 0.0f }, Rotation{ 0.0f }, Scale{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) {}
		TransformComponent(const TransformComponent&) = default;

	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
		SpriteRendererComponent(const SpriteRendererComponent&) = default;

	};

}