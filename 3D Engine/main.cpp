#include <iostream>
#include "framework.h"

framework_app_entry_point
{
	class Engine {

	};
	//game init code
	float x,y,z, fYaw;
	float speed = 100;

	float vol_l, vol_r;

	haris::AudioCapture audioCapture = haris::AudioCapture(&vol_l, &vol_r);

	haris::Game::setGameUpdate([&](float delta) {
		wchar_t charBuffer[256];

		static int frames = 0;
		static float timePassed = 0.0;
		static float theta = 0.0;

		frames++;
		timePassed += delta;
		theta += 1.0f * delta;

		if (timePassed >= 1.0f)
		{
			//swprintf(charBuffer, 256, L"FPS: %d\n", frames);
			//OutputDebugString(charBuffer);
			/*swprintf(charBuffer, 256, L"Left: %f\n", vol_l);
			OutputDebugString(charBuffer);
			swprintf(charBuffer, 256, L"Right: %f\n", vol_r);
			OutputDebugString(charBuffer);*/
			timePassed = 0.0f;
			frames = 0;
		}

		haris::Input::Position mousePosition = haris::Input::getMousePosition();
		//haris::Renderer::setPixel(10,10,{255,255,255});

		//haris::Renderer::fillRectangle({100,100,200,200},{255, 255, 255});

		//haris::Renderer::drawLine(100, 10, 200, 10, { 255, 255, 255 });

		//haris::Renderer::drawShadedTriangle({ 100 + int(x + 0.5f), 100 + int(y + 0.5f) }, { 200 + int(x + 0.5f), 250 + int(y + 0.5f) }, { 40 + int(x + 0.5f), 180 + int(y + 0.5f) }, { 255, 0, 0 });

		haris::Renderer::drawMeshes(theta, vol_l, vol_r);

		if (haris::Input::isKeyPressed(H_LEFT))
			x = 1;
		else if (haris::Input::isKeyPressed(H_RIGHT))
			x = -1;
		else
			x = 0;
		if (haris::Input::isKeyPressed(H_UP))
			y = 1;
		else if (haris::Input::isKeyPressed(H_DOWN))
			y = -1;
		else
			y = 0;

		if (haris::Input::isKeyPressed(H_D))
			fYaw = 1;
		else if (haris::Input::isKeyPressed(H_A))
			fYaw = -1;
		else
			fYaw = 0;

		if (haris::Input::isKeyPressed(H_W))
			z = 1;
		else if (haris::Input::isKeyPressed(H_S))
			z = -1;
		else
			z = 0;

		haris::Renderer::moveCamera(x, y, z, fYaw, 0, 0, delta);

		//x = mousePosition.x;
		//y = mousePosition.y;

		if (haris::Input::isKeyPressed(H_P))
			audioCapture.terminate();
	}
	);

	haris::Renderer::SetClearColor({ 0, 0, 0 });

	haris::Game::start();

	//game update logic

	//game destroy code

	return 0;
}