#pragma once

#include "Core/Base.h"
#include "Core/TimeStep.h"
#include "Graphics/EditorCamera.h"
#include "Graphics/Shader.h"
#include "Graphics/Model.h"
#include "Graphics/CubeTexture.h" // TODO: Temp...

#include <entt/entt.hpp>

namespace Aurora {

	class Entity;

	class Scene : public RefCountedObject
	{
	public:
		Scene(const std::string& debugName);
		~Scene();

		static Ref<Scene> Create(const std::string& debugName = "Scene");

		Entity CreateEntity(const char* name = "");
		Entity CopyEntity(Entity entity);
		void DestroyEntity(Entity entity);
		void Clear();
		inline size_t Size() const { return m_Registry.size(); }

		void OnUpdateEditor(TimeStep ts, EditorCamera& camera, glm::vec3 puh); // TODO: TEMPORARY!!!!!!!!!
		void OnUpdateRuntime(TimeStep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		// This a conveniance function just in case
		Entity GetPrimaryCameraEntity();

		inline const std::string& GetName() const { return m_Name; }
		inline void SetName(const std::string& name) { m_Name = name; }

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component); // If we have custom components this will not work since it will need the proper overload which is not provided

	private:
		std::string m_Name = "Untitled Scene";

		Ref<Shader> m_ModelShader; // TODO: These are both Temp...
		Ref<CubeTexture> m_EnvironmentMap;

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class EditorLayer;
		friend class SceneSerializer;

	};

}