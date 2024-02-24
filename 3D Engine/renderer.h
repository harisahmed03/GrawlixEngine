#pragma once

#include <Windows.h>
#include <stdint.h>
#include <vector>
#include <cmath>
#include <fstream>
#include <strstream>
#include "input.h"

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

	struct Point3D {
		vec3d coordinates;
		float h = 1.0f;
		vec3d normal;
	};

	struct triangle {
		Point3D p[3];
		RGBColor fillColor = { 255, 255, 255 };
		RGBColor outlineColor = { 255, 255, 255 };
	};

	struct mesh {
		std::vector<triangle> tris;
		std::vector<triangle> trisTransformed;
		bool hasChanged = true;	//If any transform has been applied, retransform

		vec3d coordinates = { 0, 0, 0 };
		vec3d rotation = { 0, 0, 0 };
		vec3d scale = { 1, 1, 1 };

		bool linkVolume = false;
		bool linkFrequency = false;
		bool linkPitch = false;
		bool linkTheta = false;

		bool drawWireframe = false;
		bool drawFilled = false;
		bool drawShaded = false;

		bool moveMesh = false;

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
					triangle temp;
					temp.p[0].coordinates = verts[f[0] - 1];
					temp.p[1].coordinates = verts[f[1] - 1];
					temp.p[2].coordinates = verts[f[2] - 1];
					tris.push_back(temp);
				}
			}
			return true;
		}

		void randomizeTriColors() {
			for (int i = 0; i < tris.size(); i++) {
				RGBColor f = { (uint8_t)(std::rand() % 255), (uint8_t)(std::rand() % 255), (uint8_t)(std::rand() % 255) };
				RGBColor o = { (uint8_t)(std::rand() % 255), (uint8_t)(std::rand() % 255), (uint8_t)(std::rand() % 255) };
				tris.at(i).fillColor = f;
				tris.at(i).outlineColor = o;
			}
		}

		void setTriColors(RGBColor fillColor, RGBColor outlineColor) {
			for (int i = 0; i < tris.size(); i++) {
				tris.at(i).fillColor = fillColor;
				tris.at(i).outlineColor = outlineColor;
			}
		}
	};

	struct Camera {
		vec3d vCamera = { 0,10,-15 };
		vec3d vLookDir;
		float horizontalYaw = 0;
		float verticalYaw = 0;
		float moveSpeed = 30;
		float lookSpeed = 200;
		bool lockMovement = false;
		bool mouseControls = false;
	};

	struct DirectionalLight {
		vec3d directionalVector;
		float ambientLightingIntensity = 0.2;
		bool hasChanged = true;
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

		static void drawFilledTriangle(triangle tri, const RGBColor& color);

		static void drawShadedTriangle(triangle tri, const RGBColor& color);

		static void drawFilledRectangle(const Rect& rect, const RGBColor& color);

		static void display2DFrequencyBars(float* freq);

		static void transformTris(mesh& myMesh, float& delta, float& vol_l, float& vol_r);

		static void display3DFrequencyBars(mesh& barMesh, mat4x4& matView, float& delta, float& vol_l, float& vol_r, float* freq, int& numBars);

		static void draw3dMesh(mesh& myMesh, mat4x4& matView, float& delta, float& vol_l, float& vol_r);

		static void RenderScene(float& delta, float& vol_l, float& vol_r, float* freq, int& numBars, float& hertz);

		static mat4x4 GetCameraViewMatrix(float deltaTime);

		static void RotateDirectionalLight(vec3d rotation);

		static void MoveMesh(mesh& myMesh, float& deltaTime);

		inline static void setScene();
		
	private:
		static void myPrint(std::string message) {
			wchar_t charBuffer[256];
			swprintf(charBuffer, 256, L"%s\n", message);
			OutputDebugString(charBuffer);
		}

		static void clearDepthBuffer(float* pDepthBuffer, int screenWidth, int screenHeight) {
			for (int i = 0; i < screenHeight * screenWidth; i++) {
				pDepthBuffer[i] = 0;
			}
		}
		static std::vector<float> interpolate(int i0, float d0, int i1, float d1) {
			if (i0 == i1 || i1 < i0) {
				return std::vector<float>{d0};
			}

			float a = (d1 - d0) / (static_cast<float>(i1) - static_cast<float>(i0));
			float d = d0;

			std::vector<float> values;
			values.reserve(i1 - i0);

			for (int i = 0; i < i1 - i0; i++) {
				values.push_back(d);
				d += a;
			}

			return values;
		}

		static std::vector<float> Bresenham(int ya, int xa, int yb, int xb)
		{
			std::vector<float> values;
			
			int dx = abs(xa - xb), dy = abs(ya - yb);
			int p = 2 * dy - dx;
			int twoDy = 2 * dy, twoDyDx = 2 * (dy - dx);
			int x, y, xEnd;
			/*Determine which points to start and End */
			if (xa > xb) {
				x = xb;
				y = yb;
				xEnd = xa;
			}
			else {
				x = xa; y = ya; xEnd = xb;
			}
			values.push_back(x);
			while (x < xEnd) {
				x++;
				if (p < 0) {
					p = p + twoDy;
				}
				else {
					y++;
					p = p + twoDyDx;
				}
				values.push_back(x);
			}
			return values;
		}

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

		static mat4x4 Matrix_MakeRotationX(float fAngleDeg)
		{
			float fAngleRad = DegreesToRadian(fAngleDeg);
			mat4x4 matrix;
			matrix.m[0][0] = 1.0f;
			matrix.m[1][1] = cosf(fAngleRad);
			matrix.m[1][2] = sinf(fAngleRad);
			matrix.m[2][1] = -sinf(fAngleRad);
			matrix.m[2][2] = cosf(fAngleRad);
			matrix.m[3][3] = 1.0f;
			return matrix;
		}

		static mat4x4 Matrix_MakeRotationY(float fAngleDeg)
		{
			float fAngleRad = DegreesToRadian(fAngleDeg);
			mat4x4 matrix;
			matrix.m[0][0] = cosf(fAngleRad);
			matrix.m[0][2] = sinf(fAngleRad);
			matrix.m[2][0] = -sinf(fAngleRad);
			matrix.m[1][1] = 1.0f;
			matrix.m[2][2] = cosf(fAngleRad);
			matrix.m[3][3] = 1.0f;
			return matrix;
		}

		static mat4x4 Matrix_MakeRotationZ(float fAngleDeg)
		{
			float fAngleRad = DegreesToRadian(fAngleDeg);
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

		static mat4x4 Matrix_MakeScale(float x, float y, float z)
		{
			mat4x4 matrix;
			matrix.m[0][0] = x;
			matrix.m[1][1] = y;
			matrix.m[2][2] = z;
			matrix.m[3][3] = 1.0f;
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

		static vec3d Vector_IntersectPlane(vec3d& plane_p, vec3d& plane_n, Point3D& lineStart, Point3D& lineEnd)
		{
			plane_n = Vector_Normalise(plane_n);
			float plane_d = -Vector_DotProduct(plane_n, plane_p);
			float ad = Vector_DotProduct(lineStart.coordinates, plane_n);
			float bd = Vector_DotProduct(lineEnd.coordinates, plane_n);
			float t = (-plane_d - ad) / (bd - ad);
			vec3d lineStartToEnd = Vector_Sub(lineEnd.coordinates, lineStart.coordinates);
			vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
			return Vector_Add(lineStart.coordinates, lineToIntersect);
		}

		static int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle& in_tri, triangle& out_tri1, triangle& out_tri2)
		{
			// Make sure plane normal is indeed normal
			plane_n = Vector_Normalise(plane_n);

			// Return signed shortest distance from point to plane, plane normal must be normalised
			auto dist = [&](vec3d& p)
				{
					vec3d n = Vector_Normalise(p);
					return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
				};

			Point3D* inside_points[3];  int nInsidePointCount = 0;
			Point3D* outside_points[3]; int nOutsidePointCount = 0;

			// Get signed distance of each point in triangle to plane
			float d0 = dist(in_tri.p[0].coordinates);
			float d1 = dist(in_tri.p[1].coordinates);
			float d2 = dist(in_tri.p[2].coordinates);

			if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
			else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
			if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
			else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
			if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
			else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

			if (nInsidePointCount == 0)
			{
				return 0; // No returned triangles are valid
			}

			if (nInsidePointCount == 3)
			{
				out_tri1 = in_tri;
			
				return 1; // Just the one returned original triangle is valid
			}

			if (nInsidePointCount == 1 && nOutsidePointCount == 2)
			{
				out_tri1.fillColor = in_tri.fillColor;
				out_tri1.outlineColor = in_tri.outlineColor;

				out_tri1.p[0].h = in_tri.p[0].h;
				out_tri1.p[1].h = in_tri.p[1].h;
				out_tri1.p[2].h = in_tri.p[2].h;

				out_tri1.p[0] = *inside_points[0];

				out_tri1.p[1].coordinates = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
				out_tri1.p[2].coordinates = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

				return 1; // Return the newly formed single triangle
			}

			if (nInsidePointCount == 2 && nOutsidePointCount == 1)
			{
				out_tri1.fillColor = in_tri.fillColor;
				out_tri1.outlineColor = in_tri.outlineColor;

				out_tri2.fillColor = in_tri.fillColor;
				out_tri2.outlineColor = in_tri.outlineColor;

				out_tri1.p[0].h = in_tri.p[0].h;
				out_tri1.p[1].h = in_tri.p[1].h;
				out_tri1.p[2].h = in_tri.p[2].h;

				out_tri2.p[0].h = in_tri.p[0].h;
				out_tri2.p[1].h = in_tri.p[1].h;
				out_tri2.p[2].h = in_tri.p[2].h;

				out_tri1.p[0] = *inside_points[0];
				out_tri1.p[1] = *inside_points[1];
				out_tri1.p[2].coordinates = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

				out_tri2.p[0] = *inside_points[1];
				out_tri2.p[1] = out_tri1.p[2];
				out_tri2.p[2].coordinates = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

				return 2; // Return two newly formed triangles which form a quad
			}
		}

		static float Vector_Distance(vec3d& vector1, vec3d& vector2) {
			return std::pow(std::pow((vector2.x - vector1.x), 2) + std::pow((vector2.y - vector1.y), 2) + std::pow((vector2.z - vector1.z), 2), 0.5f);
		}

		static float DegreesToRadian(float& degrees) {
			return degrees * (3.14159 / 180);
		}

		static double mapOneRangeToAnother(double sourceNumber, double fromA, double fromB, double toA, double toB, int decimalPrecision) {
			double deltaA = fromB - fromA;
			double deltaB = toB - toA;
			if (deltaA == 0 || deltaB == 0) {  //One set of end-points is not a range, therefore, cannot calculate a meaningful number.
				return NULL;
			}
			double scale = deltaB / deltaA;
			double negA = -1 * fromA;
			double offset = (negA * scale) + toA;
			double finalNumber = (sourceNumber * scale) + offset;
			int calcScale = (int)std::pow(10, decimalPrecision);
			return (double)std::round(finalNumber * calcScale) / calcScale;
		}

	private:
		Renderer();

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

		//Print functions for debugging
		static void print(std::string message) {
			wchar_t charBuffer[2000];
			swprintf(charBuffer, 2000, L"PortAudio error: %s\n", message);
			OutputDebugString(charBuffer);
			exit(EXIT_FAILURE);
		}

		static void printN(float message) {
			wchar_t charBuffer[2000];
			swprintf(charBuffer, 2000, L"Hertz %f\n", message);
			OutputDebugString(charBuffer);
		}

	};
}
