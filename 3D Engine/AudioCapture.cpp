#include <stdlib.h>
#include "AudioCapture.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
namespace haris {

    float* vol_l;
    float* vol_r;

    PaStream* stream;
	AudioCapture::AudioCapture(float* voll, float* volr) {
        vol_l = voll;
        vol_r = volr;
		init();
	}

    static int patestCallback(
        const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
        void* userData
    ) {
        float* in = (float*)inputBuffer;
        (void)outputBuffer;

        int dispSize = 100;
        printf("\r");

        float temp_vol_l = 0;
        float temp_vol_r = 0;

        for (unsigned long i = 0; i < framesPerBuffer * 2; i += 2) {
            temp_vol_l = AudioCapture::mymax(temp_vol_l, std::abs(in[i]));
            temp_vol_r = AudioCapture::mymax(temp_vol_r, std::abs(in[i + 1]));
        }

        *vol_l = temp_vol_l;
        *vol_r = temp_vol_r;

        /*for (int i = 0; i < dispSize; i++) {
            float barProportion = i / (float)dispSize;
            if (barProportion <= vol_l && barProportion <= vol_r) {
                printf("█");
            }
            else if (barProportion <= vol_l) {
                printf("▀");
            }
            else if (barProportion <= vol_r) {
                printf("▄");
            }
            else {
                printf(" ");
            }
        }*/

        fflush(stdout);

        return 0;
    }

    void AudioCapture::init() {
        PaError err;
        err = Pa_Initialize();
        AudioCapture::checkErr(err);

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
        int outputdevice = 3;

        PaStreamParameters inputParameters;
        PaStreamParameters outputParameters;

        memset(&inputParameters, 0, sizeof(inputParameters));
        inputParameters.channelCount = 2;
        inputParameters.device = inputdevice;
        inputParameters.hostApiSpecificStreamInfo = NULL;
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputdevice)->defaultLowInputLatency;

        memset(&outputParameters, 0, sizeof(outputParameters));
        outputParameters.channelCount = 2;
        outputParameters.device = outputdevice;
        outputParameters.hostApiSpecificStreamInfo = NULL;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputdevice)->defaultLowInputLatency;

        err = Pa_OpenStream(
            &stream,
            &inputParameters,
            &outputParameters,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paNoFlag,
            patestCallback,
            NULL
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
    }
}


