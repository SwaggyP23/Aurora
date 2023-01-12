#pragma once

#include "Core/Base.h"
#include "Core/Events/Events.h"
#include "Projects/Project.h"
#include "Scene/Scene.h"

namespace Aurora {

	class EditorPanel : public RefCountedObject
	{
	public:
		virtual ~EditorPanel() = default;

		virtual void OnImGuiRender(bool& isOpen) = 0;
		virtual void OnEvent(Event& e) {}
		virtual void OnProjectChanged(Ref<Project> project) {}
		virtual void SetSceneContext(const Ref<Scene>& scene) {}
	};

}