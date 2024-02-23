#include <iostream>
#include <memory>
#include "framework.h"

framework_app_entry_point
{
	//game init code
	
	float vol_l, vol_r;
	int numBars = 50;
	float hertz;
	float* freqDisplay = (float*)malloc(sizeof(float) * numBars);	//change to num bars

	haris::AudioCapture audioCapture = haris::AudioCapture(vol_l, vol_r, *freqDisplay, numBars, hertz);

	haris::Game::setGameUpdate([&](float delta) {
		wchar_t charBuffer[256];

		static float theta = 0.0;
		
		theta += 1.0f * delta;

		haris::Renderer::RenderScene(theta, delta, vol_l, vol_r, freqDisplay, numBars, hertz);

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