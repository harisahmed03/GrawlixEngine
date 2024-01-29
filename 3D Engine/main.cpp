#include <iostream>
#include "framework.h"

framework_app_entry_point
{
	class Engine {

	};
	//game init code
	float x = 0, y = 0;
	float speed = 100;

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
			swprintf(charBuffer, 256, L"FPS: %d\n", frames);
			OutputDebugString(charBuffer);

			timePassed = 0.0f;
			frames = 0;
		}

		haris::Input::Position mousePosition = haris::Input::getMousePosition();
		//haris::Renderer::setPixel(10,10,{255,255,255});

		//haris::Renderer::fillRectangle({100,100,200,200},{255, 255, 255});

		//haris::Renderer::drawLine(100, 10, 200, 10, { 255, 255, 255 });

		//haris::Renderer::drawShadedTriangle({ 100 + int(x + 0.5f), 100 + int(y + 0.5f) }, { 200 + int(x + 0.5f), 250 + int(y + 0.5f) }, { 40 + int(x + 0.5f), 180 + int(y + 0.5f) }, { 255, 0, 0 });

		haris::Renderer::drawMeshes(theta);

		if (haris::Input::isKeyPressed(H_A))
			x -= speed * delta;
		if (haris::Input::isKeyPressed(H_D))
			x += speed * delta;
		if (haris::Input::isKeyPressed(H_W))
			y -= speed * delta;
		if (haris::Input::isKeyPressed(H_S))
			y += speed * delta;

		//x = mousePosition.x;
		//y = mousePosition.y;
	}
	);

	haris::Renderer::SetClearColor({ 0, 0, 0 });

	haris::Game::start();

	//game update logic

	//game destroy code

	return 0;
}