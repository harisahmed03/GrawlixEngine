#define NOMINMAX
#include "renderer.h"
#include <cstdlib>
#include <algorithm>

namespace haris {
	using std::min;
	using std::max;
	void Renderer::setPixel(int x, int y, const RGBColor& color) 
	{
		BitmapBuffer& buffer = getInstance().buffer;

		//outofbounds
		if (x<0 || x>buffer.width || y<0 || y>buffer.height)
			return;

		//convert u8 colors to one u32
		uint32_t raw_color = (color.red << 16) | (color.red << 8) | (color.blue << 0);

		uint32_t* pixel = (uint32_t*)((uint8_t*)buffer.memory + x * bytes_per_pixel + y * buffer.pitch);
		*pixel = raw_color;
	}

	void Renderer::drawLine(Point a, Point b, const RGBColor& color)
	{
		//drawing lines using linear interpolation

		BitmapBuffer& buffer = getInstance().buffer;

		if (b.x<0 || b.x>buffer.width || b.y<0 ||  b.y>buffer.height || a.x<0 || a.x>buffer.width || a.y<0 || a.y>buffer.height)
			return;

		uint32_t raw_color = (color.red << 16) | (color.red << 8) | (color.blue << 0);

		uint32_t* currentPixel;
		int w = b.x - a.x;
		int h = b.y - a.y;
		int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
		if (w < 0) dx1 = -1; else if (w > 0) dx1 = 1;
		if (h < 0) dy1 = -1; else if (h > 0) dy1 = 1;
		if (w < 0) dx2 = -1; else if (w > 0) dx2 = 1;
		int longest = abs(w);
		int shortest = abs(h);
		if (!(longest > shortest)) {
			longest = abs(h);
			shortest = abs(w);
			if (h < 0) dy2 = -1; else if (h > 0) dy2 = 1;
			dx2 = 0;
		}
		int numerator = longest >> 1;
		for (int i = 0;i <= longest;i++) {
			//set pixel
			currentPixel = (uint32_t*)((uint8_t*)buffer.memory + a.x * bytes_per_pixel + a.y * buffer.pitch);
			*currentPixel = raw_color;
			numerator += shortest;
			if (!(numerator < longest)) {
				numerator -= longest;
				a.x += dx1;
				a.y += dy1;
			}
			else {
				a.x += dx2;
				a.y += dy2;
			}
		}
	}

	void Renderer::drawTriangle(Point a, Point b, Point c, const RGBColor& color){
		drawLine(a, b, color);
		drawLine(b, c, color);
		drawLine(c, a, color);
	}

	void Renderer::drawFilledTriangle(Point a, Point b, Point c, const RGBColor& color) {
		int xMin = (std::min({ a.x, b.x, c.x }));
		int xMax = (std::max({ a.x, b.x, c.x }));
		int yMin = (std::min({ a.y, b.y, c.y }));
		int yMax = (std::max({ a.y, b.y, c.y }));

		for (int y = yMin; y < yMax; y++)
			for(int x = xMin; x<xMax; x++)
				if (isInside(a.x, a.y, b.x, b.y, c.x, c.y, x, y)) {
					setPixel(x, y, color);
				}
	}

	float Renderer::area(int x1, int y1, int x2, int y2, int x3, int y3)
	{
		return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
	}

	/* A function to check whether point P(x, y) lies inside the triangle formed
	   by A(x1, y1), B(x2, y2) and C(x3, y3) */
	bool Renderer::isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
	{
		/* Calculate area of triangle ABC */
		float A = area(x1, y1, x2, y2, x3, y3);

		/* Calculate area of triangle PBC */
		float A1 = area(x, y, x2, y2, x3, y3);

		/* Calculate area of triangle PAC */
		float A2 = area(x1, y1, x, y, x3, y3);

		/* Calculate area of triangle PAB */
		float A3 = area(x1, y1, x2, y2, x, y);

		/* Check if sum of A1, A2 and A3 is same as A */
		return (A == A1 + A2 + A3);
	}

	void Renderer::fillRectangle(const Rect& rect, const RGBColor& color) 
	{
		BitmapBuffer& buffer = getInstance().buffer;

		int minX = rect.x;
		int minY = rect.y;
		int maxX = rect.x + rect.width;
		int maxY = rect.y + rect.height;

		//outofbounds
		if (minX < 0) minX = 0;
		if (minY < 0) minY = 0;
		if (maxX > buffer.width) maxX = buffer.width;
		if (maxY > buffer.height) maxY = buffer.height;

		//convert u8 colors to one u32
		uint32_t raw_color = (color.red << 16) | (color.green << 8) | (color.blue << 0);

		uint8_t* row = (uint8_t*)buffer.memory + minX * bytes_per_pixel + minY * buffer.pitch;
		for (int y = minY; y < maxY; y++) {
			uint32_t* pixel = (uint32_t*)row;
			for (int x = minX; x < maxX; x++) {
				*pixel++ = raw_color;
			}
			row += buffer.pitch;
		}

	}

	void Renderer::getWindowDimensions(int* outWidth, int* outHeight) {
		RECT clientRect;
		GetClientRect(getInstance().windowHandle, &clientRect);

		*outWidth = clientRect.right - clientRect.left;
		*outHeight = clientRect.bottom - clientRect.top;
	}

	void Renderer::resizeFrameBuffer(int width, int height) {
		BitmapBuffer& buffer = getInstance().buffer;

		if (buffer.memory) {
			VirtualFree(buffer.memory, 0, MEM_RELEASE);
		}

		buffer.width = width;
		buffer.height = height;

		buffer.info.bmiHeader.biSize = sizeof(buffer.info.bmiHeader);
		buffer.info.bmiHeader.biWidth = buffer.width;
		buffer.info.bmiHeader.biHeight = -(buffer.height);
		buffer.info.bmiHeader.biPlanes = 1;
		buffer.info.bmiHeader.biBitCount = 32;
		buffer.info.bmiHeader.biCompression = BI_RGB;

		int bufferMemorySize = buffer.width * buffer.height * bytes_per_pixel;
		buffer.memory = VirtualAlloc(0, bufferMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		buffer.pitch = buffer.width * bytes_per_pixel;
	}

	void Renderer::coppyBufferToWindow(HDC deviceContext, int windowWidth, int windowHeight) {
		BitmapBuffer& buffer = getInstance().buffer;

		StretchDIBits(
			deviceContext,
			0, 0, windowWidth, windowHeight,
			0, 0, buffer.width, buffer.height,
			buffer.memory,
			&(buffer.info),
			DIB_RGB_COLORS,
			SRCCOPY
		);
	}

	void Renderer::clear() {
		BitmapBuffer& buffer = getInstance().buffer;
		fillRectangle({ 0, 0, buffer.width, buffer.height }, getInstance().clearColor);
	}
}