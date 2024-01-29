#pragma once

#include <Windows.h>
#include <stdint.h>
#include <vector>
#include <cmath>
#include <fstream>
#include <strstream>

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
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 1;
	};

	struct triangle {
		vec3d p[3];
	};

	struct mesh {
		std::vector<triangle> tris;
		RGBColor fillColor;
		RGBColor outlineColor;

		bool loadFromObjectFile(std::string sFilename)
		{
			std::ifstream f(sFilename);
			if (!f.is_open())
				return false;

			// Local cache of verts
			std::vector<vec3d> verts;

			while (!f.eof())
			{
				char line[128];
				f.getline(line, 128);

				std::strstream s;
				s << line;

				char junk;

				if (line[0] == 'v')
				{
					vec3d v;
					s >> junk >> v.x >> v.y >> v.z;
					verts.push_back(v);
				}

				if (line[0] == 'f')
				{
					int f[3];
					s >> junk >> f[0] >> f[1] >> f[2];
					tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
				}
			}

			return true;
		}
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

		static void draw3dMesh(mesh myMesh, float fElapsedTime);

		static void drawMeshes(float theta);

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

		static vec3d Matrix_MultiplyVector(mat4x4& m, vec3d& i)
		{
			vec3d v;
			v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
			v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
			v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
			v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
			return v;
		}

		static mat4x4 Matrix_MakeIdentity()
		{
			mat4x4 matrix;
			matrix.m[0][0] = 1.0f;
			matrix.m[1][1] = 1.0f;
			matrix.m[2][2] = 1.0f;
			matrix.m[3][3] = 1.0f;
			return matrix;
		}

		static mat4x4 Matrix_MakeRotationX(float fAngleRad)
		{
			mat4x4 matrix;
			matrix.m[0][0] = 1.0f;
			matrix.m[1][1] = cosf(fAngleRad);
			matrix.m[1][2] = sinf(fAngleRad);
			matrix.m[2][1] = -sinf(fAngleRad);
			matrix.m[2][2] = cosf(fAngleRad);
			matrix.m[3][3] = 1.0f;
			return matrix;
		}

		static mat4x4 Matrix_MakeRotationY(float fAngleRad)
		{
			mat4x4 matrix;
			matrix.m[0][0] = cosf(fAngleRad);
			matrix.m[0][2] = sinf(fAngleRad);
			matrix.m[2][0] = -sinf(fAngleRad);
			matrix.m[1][1] = 1.0f;
			matrix.m[2][2] = cosf(fAngleRad);
			matrix.m[3][3] = 1.0f;
			return matrix;
		}

		static mat4x4 Matrix_MakeRotationZ(float fAngleRad)
		{
			mat4x4 matrix;
			matrix.m[0][0] = cosf(fAngleRad);
			matrix.m[0][1] = sinf(fAngleRad);
			matrix.m[1][0] = -sinf(fAngleRad);
			matrix.m[1][1] = cosf(fAngleRad);
			matrix.m[2][2] = 1.0f;
			matrix.m[3][3] = 1.0f;
			return matrix;
		}

		static mat4x4 Matrix_MakeTranslation(float x, float y, float z)
		{
			mat4x4 matrix;
			matrix.m[0][0] = 1.0f;
			matrix.m[1][1] = 1.0f;
			matrix.m[2][2] = 1.0f;
			matrix.m[3][3] = 1.0f;
			matrix.m[3][0] = x;
			matrix.m[3][1] = y;
			matrix.m[3][2] = z;
			return matrix;
		}

		static mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
		{
			float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
			mat4x4 matrix;
			matrix.m[0][0] = fAspectRatio * fFovRad;
			matrix.m[1][1] = fFovRad;
			matrix.m[2][2] = fFar / (fFar - fNear);
			matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
			matrix.m[2][3] = 1.0f;
			matrix.m[3][3] = 0.0f;
			return matrix;
		}

		static mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2)
		{
			mat4x4 matrix;
			for (int c = 0; c < 4; c++)
				for (int r = 0; r < 4; r++)
					matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
			return matrix;
		}

		static mat4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up)
		{
			// Calculate new forward direction
			vec3d newForward = Vector_Sub(target, pos);
			newForward = Vector_Normalise(newForward);

			// Calculate new Up direction
			vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
			vec3d newUp = Vector_Sub(up, a);
			newUp = Vector_Normalise(newUp);

			// New Right direction is easy, its just cross product
			vec3d newRight = Vector_CrossProduct(newUp, newForward);

			// Construct Dimensioning and Translation Matrix	
			mat4x4 matrix;
			matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
			matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
			matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
			matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
			return matrix;

		}

		static mat4x4 Matrix_QuickInverse(mat4x4& m) // Only for Rotation/Translation Matrices
		{
			mat4x4 matrix;
			matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
			matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
			matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
			matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
			matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
			matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
			matrix.m[3][3] = 1.0f;
			return matrix;
		}

		mat4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up)
		{
			// Calculate new forward direction
			vec3d newForward = Vector_Sub(target, pos);
			newForward = Vector_Normalise(newForward);

			// Calculate new Up direction
			vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
			vec3d newUp = Vector_Sub(up, a);
			newUp = Vector_Normalise(newUp);

			// New Right direction is easy, its just cross product
			vec3d newRight = Vector_CrossProduct(newUp, newForward);

			// Construct Dimensioning and Translation Matrix	
			mat4x4 matrix;
			matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
			matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
			matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
			matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
			return matrix;
		}

		static vec3d Vector_Add(vec3d& v1, vec3d& v2)
		{
			return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
		}

		static vec3d Vector_Sub(vec3d& v1, vec3d& v2)
		{
			return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
		}

		static vec3d Vector_Mul(vec3d& v1, float k)
		{
			return { v1.x * k, v1.y * k, v1.z * k };
		}

		static vec3d Vector_Div(vec3d& v1, float k)
		{
			return { v1.x / k, v1.y / k, v1.z / k };
		}

		static float Vector_DotProduct(vec3d& v1, vec3d& v2)
		{
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
		}

		static float Vector_Length(vec3d& v)
		{
			return sqrtf(Vector_DotProduct(v, v));
		}

		static vec3d Vector_Normalise(vec3d& v)
		{
			float l = Vector_Length(v);
			return { v.x / l, v.y / l, v.z / l };
		}

		static vec3d Vector_CrossProduct(vec3d& v1, vec3d& v2)
		{
			vec3d v;
			v.x = v1.y * v2.z - v1.z * v2.y;
			v.y = v1.z * v2.x - v1.x * v2.z;
			v.z = v1.x * v2.y - v1.y * v2.x;
			return v;
		}

		static vec3d Vector_IntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& lineStart, vec3d& lineEnd)
		{
			plane_n = Vector_Normalise(plane_n);
			float plane_d = -Vector_DotProduct(plane_n, plane_p);
			float ad = Vector_DotProduct(lineStart, plane_n);
			float bd = Vector_DotProduct(lineEnd, plane_n);
			float t = (-plane_d - ad) / (bd - ad);
			vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
			vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
			return Vector_Add(lineStart, lineToIntersect);
		}

		static mesh getMeshCube() {
			mesh meshCube;
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
			meshCube.outlineColor = { 255, 255, 255 };

			return meshCube;
		}

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
