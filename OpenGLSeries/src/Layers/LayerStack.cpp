#include "OGLpch.h"
#include "LayerStack.h"

LayerStack::LayerStack()
{
}

LayerStack::~LayerStack()
{
	for (Layer* layer : m_Layers)
		delete layer;
}

void LayerStack::pushLayer(Layer* layer)
{
	m_Layers.emplace(m_Layers.begin() + m_InsertIndex, layer);
	m_InsertIndex++;
}

void LayerStack::pushOverlay(Layer* layer)
{
	m_Layers.emplace_back(layer);
}

void LayerStack::popLayer(Layer* layer)
{
	std::vector<Layer*>::iterator it = std::find(m_Layers.begin(), m_Layers.end(), layer);
	if (it != m_Layers.end())
	{
		m_Layers.erase(it);
		m_InsertIndex--;
	}
}

void LayerStack::popOverlay(Layer* layer)
{
	std::vector<Layer*>::iterator it = std::find(m_Layers.begin(), m_Layers.end(), layer);
	if (it != m_Layers.end())
		m_Layers.erase(it);
}