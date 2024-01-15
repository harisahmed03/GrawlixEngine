#pragma once
#include <windows.h>
#include <stdint.h>
#include <windowsx.h>

#define MAX_KEYS 52

#define H_A			0
#define H_B			1
#define H_C			2
#define H_D			3
#define H_E			4
#define H_F			5
#define H_G			6
#define H_H			7
#define H_I			8
#define H_J			9
#define H_K			10
#define H_L			11
#define H_M			12
#define H_N			13
#define H_O			14
#define H_P			15
#define H_Q			16
#define H_R			17
#define H_S			18
#define H_T			19
#define H_U			20
#define H_V			21
#define H_W			22
#define H_X			23
#define H_Y			24
#define H_Z			25

#define H_UP		26
#define H_DOWN		27
#define H_LEFT		28
#define H_RIGHT		29

#define H_0			30
#define H_1			31
#define H_2			32
#define H_3			33
#define H_4			34
#define H_5			35
#define H_6			36
#define H_7			37
#define H_8			38
#define H_9			39
#define H_MINUS		40
#define H_PLUS		41

#define H_SHIFT		42
#define H_CONTROL	43
#define H_ALT		44
#define H_SPACE		45
#define H_ESCAPE	46
#define H_CAPSLOCK	47
#define H_TAB		48
#define H_ENTER		49
#define H_BACKSPACE	50
#define H_TILDE		51

#define MAX_MOUSE_BUTTONS 5

#define H_MOUSE_LEFT	0
#define H_MOUSE_RIGHT	1
#define H_MOUSE_MIDDLE	2
#define H_MOUSE_X1		3
#define H_MOUSE_X2		4

namespace haris {
	class Input {

		friend LRESULT CALLBACK WindowCallback(
			HWND windowHandle,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);

	public:
		struct KeyState
		{
			bool wasDown, isDown;
		};

		struct KeyboardInputMap
		{
			KeyState keys[MAX_KEYS];
		};

		struct ButtonState
		{
			bool wasDown, isDown;
		};

		struct Position
		{
			int x, y;
		};

		struct MouseInputMap
		{
			ButtonState buttons[MAX_MOUSE_BUTTONS];
			Position position;
		};

	private:
		static KeyboardInputMap keyboard;
		static MouseInputMap mouse;

	public:
		static KeyState GetKeyState(uint32_t keycode);

		static bool isKeyPressed(uint32_t keycode);
		
		static bool isKeyReleased(uint32_t keycode);

		// returns true if key has just been pressed
		static bool wasKeyHit(uint32_t keycode);

		static Position getMousePosition();

		static bool isMouseButtonPressed(unsigned int buttonCode);

		static bool isMouseButtonReleased(unsigned int buttonCode);

		static bool wasMouseButtonHit(unsigned int buttonCode);

	private:
		static void processKeyboardInput(uint32_t keycode, bool wasDown, bool isDown);

		static void processMouseInput(WPARAM wParam, LPARAM lParam);

		static void updateMousePosition(LPARAM lParam);
	};
}

