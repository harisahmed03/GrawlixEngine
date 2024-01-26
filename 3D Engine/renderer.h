#pragma once

#include <Windows.h>
#include <stdint.h>
#include <vector>
#include <cmath>

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

	struct mat4x4 {
		float m[4][4] = { 0 };
	};

	struct vec3d {
		float x, y, z;
	};

	struct triangle {
		vec3d p[3];
	};

	struct mesh {
		std::vector<triangle> tris;
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

		static void drawShadedTriangle(Point a, Point b, Point c, const RGBColor& color, float h0 = 0.3f, float h1 = 1.0f, float h2 = 0.5f);

		static void fillRectangle(const Rect& rect, const RGBColor& color);

		static void initVars();

		static void draw3dMesh(float fElapsedTime);

	private:
		static std::vector<float> interpolate(int i0, float d0, int i1, float d1);

		static void MultiplyMatrixVector(vec3d& i, vec3d& o, mat4x4& m) {
			o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
			o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
			o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
			float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

			if (w != 0.0f)
			{
				o.x /= w; o.y /= w; o.z /= w;
			}
		};

	private:
		Renderer() { buffer = {}; clearColor = { 255, 255, 255 }; initVars(); };

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
