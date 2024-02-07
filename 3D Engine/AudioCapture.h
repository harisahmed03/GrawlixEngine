#pragma once
#include <portaudio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <cstring>
namespace haris {
	class AudioCapture {
	public:
		AudioCapture(float* voll, float* volr);
		PaError err;

		static void checkErr(PaError err) {
			if (err != paNoError) {
				printf("PortAudio error: %s\n", Pa_GetErrorText(err));
				exit(EXIT_FAILURE);
			}
		}

		void init();

		void terminate();


		static void print(std::string message) {
			wchar_t charBuffer[2000];
			swprintf(charBuffer, 2000, L"PortAudio error: %s\n", message);
			OutputDebugString(charBuffer);
			exit(EXIT_FAILURE);
		}

		static inline float mymax(float a, float b) {
			return a > b ? a : b;
		}
	};
}