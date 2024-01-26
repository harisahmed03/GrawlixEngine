#define NOMINMAX
#include "renderer.h"
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>
#include <stdint.h>

namespace haris {
	using std::min;
	using std::max;

	float fNear = 0.1f;
	float fFar = 1000.0f;
	float fFov = 90.0f;
	float fAspectRatio = 1.0f;	//CHANGE THIS TO TAKE DIMENSIONS FROM WINDOW
	float fFovRad = 1.0f / std::tanf(fFov * 0.5f / 180.0f * 3.14159);

	mat4x4 matProj;

	mesh meshCube;

	vec3d vCamera = { 0,0,0 };

	mat4x4 Renderer::getZRotationMatrix(float theta) {
		mat4x4 matRotZ;
		matRotZ.m[0][0] = cosf(theta);
		matRotZ.m[0][1] = sinf(theta);
		matRotZ.m[1][0] = -sinf(theta);
		matRotZ.m[1][1] = cosf(theta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;
		return matRotZ;
	}

	mat4x4 Renderer::getXRotationMatrix(float theta) {
		mat4x4 matRotX;
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(theta * 0.5f);
		matRotX.m[1][2] = sinf(theta * 0.5f);
		matRotX.m[2][1] = -sinf(theta * 0.5f);
		matRotX.m[2][2] = cosf(theta * 0.5f);
		matRotX.m[3][3] = 1;
		return matRotX;
	}
	
	void Renderer::draw3dMesh(float theta) {

		// Set up rotation matrices
		mat4x4 matRotZ, matRotX;
		float fTheta = theta;

		// Rotation Z
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sinf(fTheta);
		matRotZ.m[1][1] = cosf(fTheta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;

		// Rotation X
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(fTheta * 0.5f);
		matRotX.m[1][2] = sinf(fTheta * 0.5f);
		matRotX.m[2][1] = -sinf(fTheta * 0.5f);
		matRotX.m[2][2] = cosf(fTheta * 0.5f);
		matRotX.m[3][3] = 1;

		//draw triangles
		for (auto tri : meshCube.tris) {
			triangle triProjected, triTranslated, triRotatedZ, triRotatedX;

			//rotate on z axis
			MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
			MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
			MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

			//rotate on x axis
			MultiplyMatrixVector(triRotatedZ.p[0], triRotatedX.p[0], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[1], triRotatedX.p[1], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[2], triRotatedX.p[2], matRotX);

			//translate triangle
			triTranslated = triRotatedX;
			triTranslated.p[0].z = triRotatedX.p[0].z + 3.0f;
			triTranslated.p[1].z = triRotatedX.p[1].z + 3.0f;
			triTranslated.p[2].z = triRotatedX.p[2].z + 3.0f;

			vec3d normal, line1, line2;
			line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
			line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
			line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

			line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
			line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
			line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z * line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;

			float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			normal.x /= l; normal.y /= l; normal.z /= l;

			//if (normal.z < 0)
			if(normal.x * (triTranslated.p[0].x - vCamera.x) +
				normal.y * (triTranslated.p[0].y - vCamera.y) +
				normal.z * (triTranslated.p[0].z - vCamera.z) < 0.0f)
			{
				//Illumination
				vec3d light_direction = { 0.0f, 0.0f, -1.0f };
				float l = sqrtf(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z);
				light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;

				float dp = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;


				MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
				MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
				MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

				//scale into view
				triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
				triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
				triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

				triProjected.p[0].x *= 0.5f * 799.0f; triProjected.p[0].y *= 0.5f * 799.0f;
				triProjected.p[1].x *= 0.5f * 799.0f; triProjected.p[1].y *= 0.5f * 799.0f;
				triProjected.p[2].x *= 0.5f * 799.0f; triProjected.p[2].y *= 0.5f * 799.0f;

				drawFilledTriangle({ (int)triProjected.p[0].x, (int)triProjected.p[0].y },
					{ (int)triProjected.p[1].x, (int)triProjected.p[1].y },
					{ (int)triProjected.p[2].x, (int)triProjected.p[2].y }, { 255,255,255 });
				drawTriangle({ (int)triProjected.p[0].x, (int)triProjected.p[0].y },
					{ (int)triProjected.p[1].x, (int)triProjected.p[1].y },
					{ (int)triProjected.p[2].x, (int)triProjected.p[2].y }, { 255,0,255 });

			}
		}
	}
	//Create init function

	void Renderer::initVars() {
		matProj.m[0][0] = fAspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][3] = 0.0f;

		meshCube.tris = {
			// SOUTH
			{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

			// EAST                                                      
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

			// NORTH                                                     
			{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

			// WEST                                                      
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

			// TOP                                                       
			{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

			// BOTTOM                                                    
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },
		};
	}

	void Renderer::setPixel(int x, int y, const RGBColor& color) 
	{
		BitmapBuffer& buffer = getInstance().buffer;

		//outofbounds
		if (x<0 || x>buffer.width || y<0 || y>buffer.height)
			return;

		//convert u8 colors to one u32
		uint32_t raw_color = (color.red << 16) | (color.green << 8) | (color.blue << 0);

		uint32_t* pixel = (uint32_t*)((uint8_t*)buffer.memory + x * bytes_per_pixel + y * buffer.pitch);
		*pixel = raw_color;
	}

	void Renderer::drawLine(Point a, Point b, const RGBColor& color)
	{
		//drawing lines using linear interpolation

		BitmapBuffer& buffer = getInstance().buffer;

		if (b.x<0 || b.x>buffer.width || b.y<0 || b.y>buffer.height || a.x<0 || a.x>buffer.width || a.y<0 || a.y>buffer.height)
			return;
		
		if (std::abs(b.x - a.x) > std::abs(b.y - a.y)) {
			if (a.x > b.x) {
				Point temp = a;
				a = b;
				b = temp;
			}

			std::vector<float> ys = interpolate(a.x, a.y, b.x, b.y);
			for (int x = a.x; x < b.x; x++) {
				setPixel(x, (int)(0.5 + ys[x - a.x]), color);
			}
		}
		else {
			if (a.y > b.y) {
				Point temp = a;
				a = b;
				b = temp;
			}

			std::vector<float> xs = interpolate(a.y, a.x, b.y, b.x);
			for (int y = a.y; y < b.y; y++) {
				setPixel((int)(0.5 + xs[y - a.y]), y, color);
			}
		}
	}

	void Renderer::drawTriangle(Point a, Point b, Point c, const RGBColor& color){
		drawLine(a, b, color);
		drawLine(b, c, color);
		drawLine(c, a, color);
	}

	void Renderer::drawFilledTriangle(Point a, Point b, Point c, const RGBColor& color) {
		// Sort the points so that y0 <= y1 <= y2
		if (b.y < a.y) {
			std::swap(a, b);
		}
		if (c.y < a.y) {
			std::swap(a, c);
		}
		if (c.y < b.y) {
			std::swap(c, b);
		}

		std::vector<float> xab = interpolate(a.y, a.x, b.y, b.x);
		std::vector<float> xbc = interpolate(b.y, b.x, c.y, c.x);
		std::vector<float> xac = interpolate(a.y, a.x, c.y, c.x);

		xab.pop_back();
		std::vector<float> xabc;
		xabc.reserve(xab.size() + xbc.size());
		xabc.insert(xabc.end(), xab.begin(), xab.end());
		xabc.insert(xabc.end(), xbc.begin(), xbc.end());

		int m = std::floor(xabc.size() / 2);

		std::vector<float> xLeft;
		std::vector<float> xRight;
		if (xac.at(m) < xabc.at(m)) {
			xLeft = xac;
			xRight = xabc;
		}
		else {
			xRight = xac;
			xLeft = xabc;
		}

		for (int y = a.y; y < c.y-1; y++) {
			for (int x = xLeft.at(y - a.y); x < xRight.at(y - a.y); x++) {
				std::cout << x;
				std::cout << y;
				setPixel(x, y, color);
			}
		}
	}

	void Renderer::drawShadedTriangle(Point a, Point b, Point c, const RGBColor& color, float h0 , float h1, float h2) {
		// Sort the points so that y0 <= y1 <= y2
		if (b.y < a.y) {
			std::swap(a, b);
			std::swap(h0, h1);
		}
		if (c.y < a.y) {
			std::swap(a, c);
			std::swap(h0, h2);
		}
		if (c.y < b.y) {
			std::swap(c, b);
			std::swap(h2, h1);
		}

		std::vector<float> xab = interpolate(a.y, a.x, b.y, b.x);
		std::vector<float> hab = interpolate(a.y, h0, b.y, h1);

		std::vector<float> xbc = interpolate(b.y, b.x, c.y, c.x);
		std::vector<float> hbc = interpolate(b.y, h1, c.y, h2);

		std::vector<float> xac = interpolate(a.y, a.x, c.y, c.x);
		std::vector<float> hac = interpolate(a.y, h0, c.y, h2);

		xab.pop_back();
		std::vector<float> xabc;
		xabc.reserve(xab.size() + xbc.size());
		xabc.insert(xabc.end(), xab.begin(), xab.end());
		xabc.insert(xabc.end(), xbc.begin(), xbc.end());

		hab.pop_back();
		std::vector<float> habc;
		habc.reserve(hab.size() + hbc.size());
		habc.insert(habc.end(), hab.begin(), hab.end());
		habc.insert(habc.end(), hbc.begin(), hbc.end());

		int m = std::floor(xabc.size() / 2);

		std::vector<float> xLeft;
		std::vector<float> xRight;

		std::vector<float> hLeft;
		std::vector<float> hRight;

		if (xac.at(m) < xabc.at(m)) {
			xLeft = xac;
			xRight = xabc;

			hLeft = hac;
			hRight = habc;
		}
		else {
			xRight = xac;
			xLeft = xabc;

			hRight = hac;
			hLeft = habc;
		}

		float xL;
		float xR;

		for (int y = a.y; y < c.y - 1; y++) {
			xL = xLeft.at(y - a.y);
			xR = xRight.at(y - a.y);

			std::vector<float> hSegment = interpolate(xL, hLeft.at(y - a.y), xR, hRight.at(y - a.y));
			for (int x = xL; x <= xR; x++) {
				float currentH = hSegment.at(x - xL);
				RGBColor shadedColor = { (uint8_t)(color.red * currentH), (uint8_t)(color.green * currentH), (uint8_t)(color.blue * currentH) };
				setPixel(x, y, shadedColor);
			}
		}
	}

	std::vector<float> Renderer::interpolate(int i0, float d0, int i1, float d1) {
		std::vector<float> values;
		if (i0 == i1) {
			values.push_back(d0);
			return values;
		}

		float a = (d1 - d0) / (static_cast<float>(i1) - static_cast<float>(i0));
		float d = d0;

		values.reserve(i1 - i0);	//make room for elements

		for (int i = i0; i <= i1; i++) {
			values.push_back(d);
			d += a;
		}
		return values;
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