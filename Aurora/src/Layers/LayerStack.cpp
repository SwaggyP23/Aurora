#include "Aurorapch.h"
#include "LayerStack.h"

namespace Aurora {

	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_InsertIndex, layer);
		m_InsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* layer)
	{
		m_Layers.emplace_back(layer);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		std::vector<Layer*>::iterator it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			m_InsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* layer)
	{
		std::vector<Layer*>::iterator it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
			m_Layers.erase(it);
	}

}