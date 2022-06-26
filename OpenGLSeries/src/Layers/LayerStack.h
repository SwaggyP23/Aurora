#pragma once

#include "Layer.h"

class LayerStack : public Layer
{
public:
	LayerStack();
	~LayerStack();

	void pushLayer(Layer* layer);
	void pushOverlay(Layer* layer);
	void popLayer(Layer* layer);
	void popOverlay(Layer* layer);

	std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
	std::vector<Layer*>::iterator end() { return m_Layers.end(); }
	std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
	std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

private:
	std::vector<Layer*> m_Layers;
	std::vector<Layer*>::iterator m_InsertIterator;

};
