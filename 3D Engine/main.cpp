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

	haris::AudioCapture::startStream(vol_l, vol_r, freqDisplay, numBars, hertz);

	haris::Game::setGameUpdate([&](float delta) {

		haris::Renderer::RenderScene(delta, vol_l, vol_r, freqDisplay, numBars, hertz);

		
	}
	);

	haris::Renderer::SetClearColor({ 0, 0, 0 });

	haris::Game::start();

	return 0;
}