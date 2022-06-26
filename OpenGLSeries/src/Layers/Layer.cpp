#include "OGLpch.h"
#include "Layer.h"

Layer::Layer(const std::string& name)
	: m_Name(name)
{
}

Layer::~Layer()
{
}

void Layer::onAttach()
{
}

void Layer::onDetach()
{
}

void Layer::onUpdate()
{
}

void Layer::onEvent(Event& e)
{
}