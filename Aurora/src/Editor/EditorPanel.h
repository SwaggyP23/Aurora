#pragma once

#include "Core/Base.h"
#include "Core/Events/Events.h"
#include "Scene/Scene.h"

namespace Aurora {

	class EditorPanel : public RefCountedObject
	{
	public:
		virtual ~EditorPanel() = default;

		virtual void OnImGuiRender(bool& isOpen) = 0;
		virtual void OnEvent(Event& e) {}
		// TODO: Projects
		virtual void SetSceneContext(const Ref<Scene>& scene) {}
	};

}