#pragma once

#include <utility>

class Input
{
public:

	inline static bool isMouseButtonPressed(int button) { return s_Instance->isMouseButtonPressedImpl(button); }
	inline static std::pair<float, float> getMousePosition(int button) { return s_Instance->getMousePositionImpl(); }
	inline static float getMouseX() { return s_Instance->getMouseXImpl(); }
	inline static float getMouseY() { return s_Instance->getMouseYImpl(); }

	inline static bool isKeyPressed(int keycode) { return s_Instance->isKeyPressedImpl(keycode); }

protected:
	virtual bool isMouseButtonPressedImpl(int button) const = 0;
	virtual std::pair<float, float> getMousePositionImpl() const = 0;
	virtual float getMouseXImpl() const = 0;
	virtual float getMouseYImpl() const = 0;

	virtual bool isKeyPressedImpl(int button) const = 0;	

private:
	static Input* s_Instance;
};