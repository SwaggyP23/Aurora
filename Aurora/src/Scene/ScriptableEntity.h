#pragma once

#include "Entity.h"

namespace Aurora {

	// Interface-like class...
	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

	protected:
		virtual void OnCreate() {}
		virtual void OnUpdate(TimeStep ts) {}
		virtual void OnDestroy() {}

	private:
		Entity m_Entity;

		friend class Scene;

	};

}