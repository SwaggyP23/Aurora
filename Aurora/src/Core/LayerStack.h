#pragma once

#include "Layer.h"

namespace Aurora {

	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		[[nodiscard]] size_t Size() const { return m_Layers.size(); }

		Layer* operator[](uint32_t index) { return m_Layers[index]; }
		const Layer* operator[](uint32_t index) const { return m_Layers[index]; }

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
		std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		std::vector<Layer*>::const_iterator begin() const { return m_Layers.cbegin(); }
		std::vector<Layer*>::const_iterator end() const { return m_Layers.cend(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.crbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.crend(); }

	private:
		std::vector<Layer*> m_Layers;
		uint32_t m_InsertIndex = 0;

	};

}