#pragma once

#include "Input.h"

class InputImplementation : public Input
{
protected:
	virtual std::pair<float, float> getMousePositionImpl() const override;
	virtual bool isMouseButtonPressedImpl(int button) const override;
	virtual float getMouseXImpl() const override;
	virtual float getMouseYImpl() const override;

	virtual bool isKeyPressedImpl(int button) const override;
};