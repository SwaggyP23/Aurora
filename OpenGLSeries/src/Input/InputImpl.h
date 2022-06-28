#pragma once

#include "Input.h"

class InputImplementation : public Input
{
protected:
	virtual std::pair<float, float> getMousePositionImpl() const override;
	virtual bool isMouseButtonPressedImpl(int button) const override;
	virtual float getMouseXOffImpl() const override;
	virtual float getMouseYOffImpl() const override;

	virtual bool isKeyPressedImpl(int button) const override;
};