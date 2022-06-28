#pragma once

#include <utility>

class Input
{
public:

	inline static bool isMouseButtonPressed(int button) { return s_Instance->isMouseButtonPressedImpl(button); }
	inline static std::pair<float, float> getMousePosition() { return s_Instance->getMousePositionImpl(); }
	inline static float getMouseXOff() { return s_Instance->getMouseXOffImpl(); }
	inline static float getMouseYOff() { return s_Instance->getMouseYOffImpl(); }

	inline static bool isKeyPressed(int keycode) { return s_Instance->isKeyPressedImpl(keycode); }

protected:
	virtual bool isMouseButtonPressedImpl(int button) const = 0;
	virtual std::pair<float, float> getMousePositionImpl() const = 0;
	virtual float getMouseXOffImpl() const = 0;
	virtual float getMouseYOffImpl() const = 0;

	virtual bool isKeyPressedImpl(int button) const = 0;	

private:
	static Input* s_Instance;
};