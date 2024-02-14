#include <stdlib.h>
#include "AudioCapture.h"
#include <fftw3.h>
#include <cmath>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 1024
#define NUM_CHANNELS 2

#define SPECTRO_FREQ_START 20
#define SPECTRO_FREQ_END 1000

namespace haris {

    typedef struct {
        double* in;
        double* out;
        fftw_plan p;
        int startIndex;
        int spectroSize;
    } streamCallbackData;

    static streamCallbackData* spectroData;

    float* vol_l;
    float* vol_r;

    float* freqDisplay;

    PaStream* stream;
	AudioCapture::AudioCapture(float* voll, float* volr, float* freqD) {
        vol_l = voll;
        vol_r = volr;
        freqDisplay = freqD;
        freqDisplay = freqD;
		init();
	}

    static int patestCallback(
        const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
        void* userData
    ) {
        float* in = (float*)inputBuffer;
        (void)outputBuffer;
        streamCallbackData* callbackData = (streamCallbackData*)userData;

        int dispSize = 100;

        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            callbackData->in[i] = (in[i * NUM_CHANNELS]);
        }

        fftw_execute(callbackData->p);

        for (int i = 0; i < dispSize; i++) {
            //double proportion = std::pow(i / (double)dispSize, 3);
            double proportion = i / (double)dispSize;
            double frequency = callbackData->out[(int)(callbackData->startIndex + proportion * callbackData->spectroSize)];
            freqDisplay[i] = frequency;
        }
        //operate on the fourier transform

        float temp_vol_l = 0;
        float temp_vol_r = 0;

        for (unsigned long i = 0; i < framesPerBuffer * 2; i += 2) {
            temp_vol_l = AudioCapture::mymax(temp_vol_l, std::abs(in[i]));
            temp_vol_r = AudioCapture::mymax(temp_vol_r, std::abs(in[i + 1]));
        }

        *vol_l = temp_vol_l;
        *vol_r = temp_vol_r;

        return 0;
    }

    void AudioCapture::init() {
        PaError err;
        err = Pa_Initialize();
        AudioCapture::checkErr(err);

        spectroData = (streamCallbackData*)malloc(sizeof(streamCallbackData));
        spectroData->in = (double*)malloc(sizeof(double) * FRAMES_PER_BUFFER);
        spectroData->out = (double*)malloc(sizeof(double) * FRAMES_PER_BUFFER);
        if (spectroData->in == NULL || spectroData->out == NULL) {
            printf("Could not allocate spectro data\n");
            exit(EXIT_FAILURE);
        }
        spectroData->p = fftw_plan_r2r_1d(
            FRAMES_PER_BUFFER, spectroData->in,
            spectroData->out, FFTW_R2HC, FFTW_ESTIMATE
        );
        double sampleRatio = FRAMES_PER_BUFFER / (double)SAMPLE_RATE;
        spectroData->startIndex = std::ceil(sampleRatio * SPECTRO_FREQ_START);
        spectroData->spectroSize = mymin(std::ceil(sampleRatio * SPECTRO_FREQ_END), FRAMES_PER_BUFFER/2.0) - spectroData->startIndex;

        int numDevices = Pa_GetDeviceCount();
        printf("Number of devices: %d\n", numDevices);

        if (numDevices < 0) {
            printf("Error getting device count.\n");
            exit(EXIT_FAILURE);
        }
        else if (numDevices == 0) {
            printf("There are no available audio devices on this machine.\n");
            exit(EXIT_SUCCESS);
        }

        const PaDeviceInfo* deviceInfo;
        for (int i = 0; i < numDevices; i++) {
            deviceInfo = Pa_GetDeviceInfo(i);
            printf("Device %d:\n", i);
            printf("  name: %s\n", deviceInfo->name);
            printf("  maxInputChannels: %d\n", deviceInfo->maxInputChannels);
            printf("  maxOutputChannels: %d\n", deviceInfo->maxOutputChannels);
            printf("  defaultSampleRate: %f\n", deviceInfo->defaultSampleRate);
        }

        int inputdevice = 0;

        PaStreamParameters inputParameters;
        PaStreamParameters outputParameters;

        memset(&inputParameters, 0, sizeof(inputParameters));
        inputParameters.channelCount = NUM_CHANNELS;
        inputParameters.device = inputdevice;
        inputParameters.hostApiSpecificStreamInfo = NULL;
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputdevice)->defaultLowInputLatency;

        err = Pa_OpenStream(
            &stream,
            &inputParameters,
            NULL,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paNoFlag,
            patestCallback,
            spectroData
        );
        AudioCapture::checkErr(err);

        err = Pa_StartStream(stream);
        AudioCapture::checkErr(err);        
    }

    void AudioCapture::terminate() {
        err = Pa_StopStream(stream);
        AudioCapture::checkErr(err);

        err = Pa_CloseStream(stream);
        AudioCapture::checkErr(err);

        err = Pa_Terminate();
        AudioCapture::checkErr(err);

        fftw_destroy_plan(spectroData->p);
        fftw_free(spectroData->in);
        fftw_free(spectroData->out);
        free(spectroData);
    }
}


