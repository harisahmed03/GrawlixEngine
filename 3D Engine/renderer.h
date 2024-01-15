#pragma once

#include <Windows.h>
#include <stdint.h>

namespace haris
{
	struct RGBColor {
		uint8_t red, green, blue;
	};

	struct Rect {
		int x, y, width, height;
	};

	struct Point {
		int x, y;
	};

	class Renderer {

		friend LRESULT CALLBACK WindowCallback(
			HWND windowHandle,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);

		friend class Game;

		struct BitmapBuffer {
			int width, height;
			BITMAPINFO info;
			void* memory;
			int pitch; //in bytes
		};

	private:
		static const int bytes_per_pixel = 4;

		HWND windowHandle = 0;
		BitmapBuffer buffer;
		RGBColor clearColor;

	public:
		inline static void SetClearColor(const RGBColor& color) { getInstance().clearColor = color; }

		static void setPixel(int x, int y, const RGBColor& color);

		static void drawLine(Point a, Point b, const RGBColor& color);

		static void drawTriangle(Point a, Point b, Point c, const RGBColor& color);

		static void drawFilledTriangle(Point a, Point b, Point c, const RGBColor& color);

		static void fillRectangle(const Rect& rect, const RGBColor& color);

	private:
		Renderer() { buffer = {}; clearColor = { 255, 255, 255 }; };

		Renderer(const Renderer&) = delete;
		Renderer& operator = (const Renderer&) = delete;

		~Renderer() {}

		inline static Renderer& getInstance() {
			static Renderer renderer;
			return renderer;
		}

	private:
		inline static void setWindowHandle(HWND _windowHandle) { getInstance().windowHandle = _windowHandle; }

		static void getWindowDimensions(int* outWidth, int* outHeight);

		static void resizeFrameBuffer(int width, int height);

		static void coppyBufferToWindow(HDC deviceContext, int windowWidth, int windowHeight);

		static void clear();
	};
}
