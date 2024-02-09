#include <iostream>
#include "framework.h"

framework_app_entry_point
{
	//game init code
	
	float speed = 100;

	float vol_l, vol_r;
	float* freqDisplay = (float*)malloc(sizeof(float) * 100);	//change to num bars

	haris::AudioCapture audioCapture = haris::AudioCapture(&vol_l, &vol_r, freqDisplay);

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

		haris::Renderer::RenderScene(theta, delta, vol_l, vol_r, freqDisplay);


		if (haris::Input::isKeyPressed(H_P))
			audioCapture.terminate();
	}
	);

	haris::Renderer::SetClearColor({ 0, 0, 0 });

	haris::Game::start();

	return 0;
}