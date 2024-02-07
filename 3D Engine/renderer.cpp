#define NOMINMAX
#include "renderer.h"
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include <stdint.h>

namespace haris {
	using std::min;
	using std::max;

	mat4x4 matProj;

	mesh meshCube;
	std::vector<mesh> myMeshes;

	vec3d vCamera = { 0,0,0 };
	vec3d vLookDir;
	float fYaw = 0;

	float myL, myR;
	float sensitivity = 10.0f;

	float cameraMoveSpeed = 15;
	
	void Renderer::moveCamera(float x, float y, float z, float yaw, float yr, float zr, float theta) {
		vec3d temp = { x*cameraMoveSpeed * theta, y*cameraMoveSpeed * theta, z* cameraMoveSpeed * theta,0 };
		vCamera = Vector_Add(vCamera, temp);
		fYaw += yaw * (cameraMoveSpeed/4) * theta;
	}
	void Renderer::drawMeshes(float theta, float vol_l, float vol_r) {
		myL = vol_l;
		myR = vol_r;
		draw3dMesh(meshCube, theta);
	}
	void Renderer::draw3dMesh(mesh myMesh, float theta) {

		mat4x4 matRotZ = Matrix_MakeRotationZ(theta);
		mat4x4 matRotX = Matrix_MakeRotationX(theta/2);

		mat4x4 matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 15.0f);

		float volStuff = 1.0f + (myL * sensitivity);
		mat4x4 matScale = Matrix_MakeScale(volStuff, volStuff, volStuff);
		mat4x4 matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
		matWorld = Matrix_MultiplyMatrix(matWorld, matScale);
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

		vec3d vUp = { 0,1,0 };
		vec3d vTarget = { 0, 0, 1 };
		
		mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
		vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
		vTarget = Vector_Add(vCamera, vLookDir);

		mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

		mat4x4 matView = Matrix_QuickInverse(matCamera);

		std::vector<triangle> vecTrianglesToRaster;

		//draw triangles
		for (auto tri : myMesh.tris) {
			triangle triProjected, triTransformed, triViewed;

			triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
			triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
			triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

			vec3d normal, line1, line2;
			line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
			normal = Vector_CrossProduct(line1, line2);
			normal = Vector_Normalise(normal);

			vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

			if(Vector_DotProduct(normal, vCameraRay) < 0.0f)
			{
				triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
				triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
				triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

				int nClippedTriangles = 0;
				triangle clipped[2];
				nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

				for (int n = 0; n < nClippedTriangles; n++)
				{
					//Apply projection matrix
					triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
					triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
					triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);

					//Normalize
					triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
					triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
					triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

					vec3d vOffsetView = { 1,1,0 };

					triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
					triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
					triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

					float myScale = 799.0f;
					triProjected.p[0].x *= 0.5f * myScale; triProjected.p[0].y *= 0.5f * myScale;
					triProjected.p[1].x *= 0.5f * myScale; triProjected.p[1].y *= 0.5f * myScale;
					triProjected.p[2].x *= 0.5f * myScale; triProjected.p[2].y *= 0.5f * myScale;

					triProjected.fillColor = tri.fillColor;
					triProjected.outlineColor = tri.outlineColor;

					vecTrianglesToRaster.push_back(triProjected);
				}
			}
		}

		// Sort triangles from back to front
		std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
				float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return z1 > z2;
			});
		for (auto& triToRaster : vecTrianglesToRaster)
		{
			// Clip triangles against all four screen edges, this could yield
			// a bunch of triangles, so create a queue that we traverse to 
			//  ensure we only test new triangles generated against planes
			triangle clipped[2];
			std::list<triangle> listTriangles;

			// Add initial triangle
			listTriangles.push_back(triToRaster);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles > 0)
				{
					// Take triangle from front of queue
					triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					// Clip it against a plane. We only need to test each 
					// subsequent plane, against subsequent new triangles
					// as all triangles after a plane clip are guaranteed
					// to lie on the inside of the plane. I like how this
					// comment is almost completely and utterly justified
					switch (p)
					{
					case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)800 - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)800 - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					}

					// Clipping may yield a variable number of triangles, so
					// add these new ones to the back of the queue for subsequent
					// clipping against next planes
					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(clipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}

			for (auto& t : listTriangles) {

				//drawShadedTriangle({ (int)t.p[0].x, (int)t.p[0].y }, { (int)t.p[1].x, (int)t.p[1].y }, { (int)t.p[2].x, (int)t.p[2].y }, t.fillColor);

				//Wireframe
				drawTriangle({ (int)t.p[0].x, (int)t.p[0].y },
					{ (int)t.p[1].x, (int)t.p[1].y },
					{ (int)t.p[2].x, (int)t.p[2].y }, t.outlineColor);
			}
		}
	}
	

	void Renderer::setPixel(int x, int y, const RGBColor& color) 
	{
		BitmapBuffer& buffer = getInstance().buffer;

		//outofbounds
		if (x<=0 || x>=buffer.width || y<=0 || y>=buffer.height)
			return;

		x = buffer.width - x;
		y = buffer.height - y;

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

		BitmapBuffer& buffer = getInstance().buffer;

		for (int y = a.y; y < c.y-1; y++) {
			for (int x = xLeft.at(y - a.y); x < xRight.at(y - a.y); x++) {
				//setPixel(x, y, color);
				//convert u8 colors to one u32
				uint32_t raw_color = (color.red << 16) | (color.green << 8) | (color.blue << 0);
				int tempX = buffer.width - x;
				int tempY = buffer.height - y;
				if (tempX>0 && tempX<buffer.width && tempY>0 && tempY<buffer.height) {
					uint32_t* pixel = (uint32_t*)((uint8_t*)buffer.memory + tempX * bytes_per_pixel + tempY * buffer.pitch);
					*pixel = raw_color;
				}
			}
		}
	}

	void Renderer::drawShadedTriangle(Point a, Point b, Point c, RGBColor& color, float h0 , float h1, float h2) {
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

		BitmapBuffer& buffer = getInstance().buffer;

		for (int y = a.y; y < c.y - 1; y++) {
			xL = xLeft.at(y - a.y);
			xR = xRight.at(y - a.y);

			std::vector<float> hSegment = interpolate(xL, hLeft.at(y - a.y), xR, hRight.at(y - a.y));
			for (int x = xL; x <= xR; x++) {
				float currentH = hSegment.at(x - xL);
				RGBColor shadedColor = { (uint8_t)(color.red * currentH), (uint8_t)(color.green * currentH), (uint8_t)(color.blue * currentH) };
				uint32_t raw_color = (shadedColor.red << 16) | (shadedColor.green << 8) | (shadedColor.blue << 0);
				int tempX = buffer.width - x;
				int tempY = buffer.height - y;
				if (tempX > 0 && tempX < buffer.width && tempY>0 && tempY < buffer.height) {
					uint32_t* pixel = (uint32_t*)((uint8_t*)buffer.memory + tempX * bytes_per_pixel + tempY * buffer.pitch);
					*pixel = raw_color;
				}
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

		values.reserve(i1 - i0 + 1);	//make room for elements

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

	//Create init function
	void Renderer::initVars() {
		matProj = Matrix_MakeProjection(90.0f, 1.0f, 0.1f, 1000.0f);

		meshCube.loadFromObjectFile("Cube.txt");
		if (meshCube.tris.size() == 0) {
			exit(0);
		}

		meshCube.randomizeTriColors();
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