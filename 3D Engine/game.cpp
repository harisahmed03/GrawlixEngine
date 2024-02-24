#include "game.h"
#include "renderer.h"
#include "input.h"
#include "AudioCapture.h"

namespace haris
{
	Game::Game()
	{
		// init window properties to default values
		windowTitle = L"Haris's App";
		windowWidth = 800;
		windowHeight = 800;
		timePassed = 0.0;
		frames = 0;
	}

	//handles window events
	LRESULT CALLBACK WindowCallback(
		HWND windowHandle,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
	{
		LRESULT result = 0;

		switch (message) {
		case WM_CLOSE: {
			Game::getInstance().running = false;
			AudioCapture::terminate();
			OutputDebugString(L"Window close\n");
		}break;

		case WM_DESTROY: {
			Game::getInstance().running = false;
			AudioCapture::terminate();
			OutputDebugString(L"Window destroyed\n");
		}break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32_t VKCode = wParam;

			bool wasDown = (lParam & (1 << 30)) != 0;
			bool isDown = (lParam & (1 << 31)) == 0;

			Input::processKeyboardInput(VKCode, wasDown, isDown);
		}break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		{
			Input::processMouseInput(wParam, lParam);
		} break;

		case WM_MOUSEMOVE:
		{
			Input::updateMousePosition(lParam);
		} break;

		case WM_PAINT: {
			OutputDebugString(L"Window paint\n");
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(windowHandle, &paint);
			int width, height;
			Renderer::getWindowDimensions(&width, &height);
			EndPaint(windowHandle, &paint);
		}break;

		case WM_ERASEBKGND: {
			return (LRESULT)1;
		}

		default:
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}

		return result;
	}

	void Game::startWindow() {
		Renderer::resizeFrameBuffer(windowWidth, windowHeight);

		const wchar_t* className = L"haris_window";

		WNDCLASS windowClass = {};

		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowCallback;
		windowClass.hInstance = hInstance;
		windowClass.lpszClassName = className;

		if (!RegisterClass(&windowClass)) {
			OutputDebugString(L"Failed to register window class\n");
			return;
		}

		DWORD windowStyle = WS_MAXIMIZEBOX | WS_SYSMENU | WS_CAPTION;

		RECT windowRect = {};
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = windowWidth;
		windowRect.bottom = windowHeight;

		AdjustWindowRect(&windowRect, windowStyle, false);


		windowHandle = CreateWindowEx(
			0,
			className,
			windowTitle.c_str(),
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			0,
			0,
			hInstance,
			0
		);

		if (windowHandle) {
			OutputDebugString(L"GAME INIT\n");
			running = true;

			Renderer::setWindowHandle(windowHandle);

			//init the clock
			LARGE_INTEGER cpu_frequency;
			QueryPerformanceFrequency(&cpu_frequency);

			LARGE_INTEGER last_counter;
			QueryPerformanceCounter(&last_counter);

			while (running) {

				LARGE_INTEGER current_counter;
				QueryPerformanceCounter(&current_counter);

				int64_t counter_elapsed = current_counter.QuadPart - last_counter.QuadPart;

				float delta = (float)counter_elapsed / (float)cpu_frequency.QuadPart;	//in seconds

				last_counter = current_counter;

				timePassed += delta;
				frames++;

				//process windows messages
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
					if (message.message == WM_QUIT)
						running = false;

					TranslateMessage(&message);
					DispatchMessage(&message);	//send message to WindowProc (WindowCallBack)
				}

				//update and render

				Renderer::clear();

				getInstance().update(delta);

				HDC deviceContext = GetDC(windowHandle);

				int width, height;
				Renderer::getWindowDimensions(&width, &height);

				if (timePassed >= 1.0f)
				{
					wsprintfW(buffer, L"%d", frames);		
					frames = 0;
					timePassed = 0;
				}	
				RECT rc;
				GetClientRect(windowHandle, &rc);
				SetBkMode(deviceContext, TRANSPARENT);
				SetTextColor(deviceContext, 0x0000FF00);
				DrawText(deviceContext, buffer, -1, &rc, DT_TOP | DT_LEFT);

				Renderer::coppyBufferToWindow(deviceContext, width, height);

				ReleaseDC(windowHandle, deviceContext);
			}
		}
		else {
			OutputDebugString(L"Failed to create window handle\n");
		}
	}
}