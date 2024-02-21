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
		AudioCapture(float& vol_l, float& vol_r, float& freqDisplay, int& numBars, float& hertz);
		PaError err;

		static void checkErr(PaError err) {
			if (err != paNoError) {
				printf("PortAudio error: %s\n", Pa_GetErrorText(err));
				exit(EXIT_FAILURE);
			}
		}
		static int patestCallback(
			const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
			void* userData
		);

		void terminate();

		static void changeNumBars(int& numBars);

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

		static inline float mymax(float a, float b) {
			return a > b ? a : b;
		}

		static inline float mymin(float a, float b) {
			return a < b ? a : b;
		}
	};
}