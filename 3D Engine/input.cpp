#include "input.h"

namespace haris {
	Input::KeyboardInputMap Input::keyboard;
	Input::MouseInputMap Input::mouse;

	Input::KeyState Input::GetKeyState(uint32_t keycode) {
		return keyboard.keys[keycode];
	}
	bool Input::isKeyPressed(uint32_t keycode) {
		return keyboard.keys[keycode].isDown;
	}
	bool Input::isKeyReleased(uint32_t keycode) {
		return !keyboard.keys[keycode].isDown;
	}

	//returns true if key has just been pressed
	bool Input::wasKeyHit(uint32_t keycode) {
		return ((!keyboard.keys[keycode].wasDown) && keyboard.keys[keycode].isDown);
	}

	void Input::processKeyboardInput(uint32_t VKCode, bool wasDown, bool isDown) {
		if (wasDown != isDown) {
			if (VKCode >= 'A' && VKCode <= 'Z') {
				uint32_t newKeyCode = VKCode - 'A';
				keyboard.keys[newKeyCode].isDown = isDown;
				keyboard.keys[newKeyCode].wasDown = wasDown;
			}
			else if (VKCode == VK_UP)
			{
				keyboard.keys[H_UP].isDown = isDown;
				keyboard.keys[H_UP].wasDown = wasDown;
			}
			else if (VKCode == VK_DOWN)
			{
				keyboard.keys[H_DOWN].isDown = isDown;
				keyboard.keys[H_DOWN].wasDown = wasDown;
			}
			else if (VKCode == VK_LEFT)
			{
				keyboard.keys[H_LEFT].isDown = isDown;
				keyboard.keys[H_LEFT].wasDown = wasDown;
			}
			else if (VKCode == VK_RIGHT)
			{
				keyboard.keys[H_RIGHT].isDown = isDown;
				keyboard.keys[H_RIGHT].wasDown = wasDown;
			}
			else if (VKCode >= '0' && VKCode <= '9')
			{
				uint32_t H_keycode = VKCode - '0' + H_0;
				keyboard.keys[H_keycode].isDown = isDown;
				keyboard.keys[H_keycode].wasDown = wasDown;
			}
			else if (VKCode == VK_OEM_MINUS)
			{
				keyboard.keys[H_MINUS].isDown = isDown;
				keyboard.keys[H_MINUS].wasDown = wasDown;
			}
			else if (VKCode == VK_OEM_PLUS)
			{
				keyboard.keys[H_PLUS].isDown = isDown;
				keyboard.keys[H_PLUS].wasDown = wasDown;
			}
			else if (VKCode == VK_SHIFT)
			{
				keyboard.keys[H_SHIFT].isDown = isDown;
				keyboard.keys[H_SHIFT].wasDown = wasDown;
			}
			else if (VKCode == VK_CONTROL)
			{
				keyboard.keys[H_CONTROL].isDown = isDown;
				keyboard.keys[H_CONTROL].wasDown = wasDown;
			}
			else if (VKCode == VK_MENU)
			{
				keyboard.keys[H_ALT].isDown = isDown;
				keyboard.keys[H_ALT].wasDown = wasDown;
			}
			else if (VKCode == VK_SPACE)
			{
				keyboard.keys[H_SPACE].isDown = isDown;
				keyboard.keys[H_SPACE].wasDown = wasDown;
			}
			else if (VKCode == VK_ESCAPE)
			{
				keyboard.keys[H_ESCAPE].isDown = isDown;
				keyboard.keys[H_ESCAPE].wasDown = wasDown;
			}
			else if (VKCode == VK_CAPITAL)
			{
				keyboard.keys[H_CAPSLOCK].isDown = isDown;
				keyboard.keys[H_CAPSLOCK].wasDown = wasDown;
			}
			else if (VKCode == VK_TAB)
			{
				keyboard.keys[H_TAB].isDown = isDown;
				keyboard.keys[H_TAB].wasDown = wasDown;
			}
			else if (VKCode == VK_RETURN)
			{
				keyboard.keys[H_ENTER].isDown = isDown;
				keyboard.keys[H_ENTER].wasDown = wasDown;
			}
			else if (VKCode == VK_BACK)
			{
				keyboard.keys[H_BACKSPACE].isDown = isDown;
				keyboard.keys[H_BACKSPACE].wasDown = wasDown;
			}
			else if (VKCode == VK_OEM_3)
			{
				keyboard.keys[H_TILDE].isDown = isDown;
				keyboard.keys[H_TILDE].wasDown = wasDown;
			}
		}
	}

	Input::Position Input::getMousePosition() {
		return mouse.position;
	}

	bool Input::isMouseButtonPressed(unsigned int buttonCode) {
		return mouse.buttons[buttonCode].isDown;
	}

	bool Input::isMouseButtonReleased(unsigned int buttonCode) {
		return !mouse.buttons[buttonCode].isDown;
	}

	bool Input::wasMouseButtonHit(unsigned int buttonCode) {
		return ((!mouse.buttons[buttonCode].wasDown) && (mouse.buttons[buttonCode].isDown));
	}

	void Input::processMouseInput(WPARAM wParam, LPARAM lParam) {
		mouse.buttons[H_MOUSE_LEFT].wasDown = mouse.buttons[H_MOUSE_LEFT].isDown;
		mouse.buttons[H_MOUSE_RIGHT].wasDown = mouse.buttons[H_MOUSE_RIGHT].isDown;
		mouse.buttons[H_MOUSE_MIDDLE].wasDown = mouse.buttons[H_MOUSE_MIDDLE].isDown;
		mouse.buttons[H_MOUSE_X1].wasDown = mouse.buttons[H_MOUSE_X1].isDown;
		mouse.buttons[H_MOUSE_X2].wasDown = mouse.buttons[H_MOUSE_X2].isDown;

		mouse.buttons[H_MOUSE_LEFT].isDown = wParam & MK_LBUTTON;
		mouse.buttons[H_MOUSE_RIGHT].isDown = wParam & MK_RBUTTON;
		mouse.buttons[H_MOUSE_MIDDLE].isDown = wParam & MK_MBUTTON;
		mouse.buttons[H_MOUSE_X1].isDown = wParam & MK_XBUTTON1;
		mouse.buttons[H_MOUSE_X2].isDown = wParam & MK_XBUTTON2;

		updateMousePosition(lParam);
	}

	void Input::updateMousePosition(LPARAM lParam) {
		mouse.position.x = GET_X_LPARAM(lParam);
		mouse.position.y = GET_Y_LPARAM(lParam);
	}

	void Input::setMousePosition(Position newPos) {
		SetCursorPos(newPos.x, newPos.y);
	}
}