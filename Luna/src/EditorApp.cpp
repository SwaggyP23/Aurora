#include <Aurora.h>
#include <Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Aurora {

	class Luna : public Application
	{
	public:
		Luna()
			: Application("Luna")
		{
			pushLayer(new EditorLayer());
		}

		~Luna()
		{
		}
	};

	Application* CreateApplication()
	{
		return new Luna();
	}

}