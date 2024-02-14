#include <iostream>
#include <memory>
#include "framework.h"

framework_app_entry_point
{
	//game init code
	float vol_l, vol_r;
	int numBars = 10;
	float* freqDisplay = (float*)malloc(sizeof(float) * numBars);	//change to num bars

	haris::AudioCapture audioCapture = haris::AudioCapture(vol_l, vol_r, *freqDisplay, numBars);

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

		haris::Renderer::RenderScene(theta, delta, vol_l, vol_r, freqDisplay, numBars);


		if (haris::Input::isKeyPressed(H_P))
			audioCapture.terminate();
		if (haris::Input::isKeyPressed(H_K)) {
			numBars = numBars - 1;
			audioCapture.changeNumBars(numBars);
		}	
		else if (haris::Input::isKeyPressed(H_L)) {
			numBars = numBars + 1;
			audioCapture.changeNumBars(numBars);
		}
	}
	);

	haris::Renderer::SetClearColor({ 0, 0, 0 });

	haris::Game::start();

	return 0;
}