#include <stdlib.h>
#include "AudioCapture.h"
#include <fftw3.h>
#include <cmath>
#include <sndfile.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 1024
#define NUM_CHANNELS 2
#define RESOLUTION SAMPLE_RATE/FRAMES_PER_BUFFER

#define SPECTRO_FREQ_START 20
#define SPECTRO_FREQ_END 1000

#define WINDOWS_DEFAULT_AUDIO_DEVICE 0

namespace haris {

    //User data that we pass to the portaudio audio callback function
    typedef struct {
        double* in;
        double* out;
        fftw_plan p;
        int startIndex;
        int spectroSize;

        float* vol_l;
        float* vol_r;
        float* freqDisplay;
        int* numBars;
        float* hertz;

    } streamCallbackData;

    static streamCallbackData* spectroData;
    PaStream* stream;

    void AudioCapture::startStream(float& vol_l, float& vol_r, float* freqDisplay, int& numBars, float& hertz) {
        PaError err;    //Initializing port audio
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

        spectroData = (streamCallbackData*)malloc(sizeof(streamCallbackData));  //User data we will send to our audio callback function
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
        spectroData->spectroSize = mymin(std::ceil(sampleRatio * SPECTRO_FREQ_END), FRAMES_PER_BUFFER / 2.0) - spectroData->startIndex;

        spectroData->vol_l = &vol_l;    //set pointers to variables, to be used by renderer
        spectroData->freqDisplay = freqDisplay;
        spectroData->vol_r = &vol_r;
        spectroData->numBars = &numBars;
        spectroData->hertz = &hertz;

        int inputdevice = WINDOWS_DEFAULT_AUDIO_DEVICE;

        PaStreamParameters inputParameters;

        memset(&inputParameters, 0, sizeof(inputParameters));   //Set input parameters
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
            microphoneAudioCallback,
            spectroData
        );
        AudioCapture::checkErr(err);

        err = Pa_StartStream(stream);   //Start executing callback function
        AudioCapture::checkErr(err);
    }

    int AudioCapture::microphoneAudioCallback(
        const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
        void* userData
    ) {
        float* in = (float*)inputBuffer;
        (void)outputBuffer;
        streamCallbackData* callbackData = (streamCallbackData*)userData;

        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            callbackData->in[i] = (in[i * NUM_CHANNELS]);
        }

        fftw_execute(callbackData->p);
        int numBars = *((streamCallbackData*)userData)->numBars;
        float* freqDisplay = ((streamCallbackData*)userData)->freqDisplay;

        for (int i = 0; i < numBars; i++) {
            //double proportion = std::pow(i / (double)numBars, 2);
            double proportion = i / (double)numBars;
            double frequency = callbackData->out[(int)(callbackData->startIndex + proportion * callbackData->spectroSize)];
            freqDisplay[i] = frequency;
        }
        //operate on the fourier transform

        float max_vol_l = 0;
        float max_vol_r = 0;

        for (unsigned long i = 0; i < framesPerBuffer * 2; i += 2) {
            if (max_vol_l < std::abs(in[i])) {
                max_vol_l = in[i];
            }
            if (max_vol_r < std::abs(in[i+1])) {
                max_vol_r = in[i];
            }
        }

        *((streamCallbackData*)userData)->vol_l = max_vol_l;
        *((streamCallbackData*)userData)->vol_r = max_vol_l;

        return 0;
    }

    void AudioCapture::terminate() {
        PaError err = Pa_StopStream(stream);
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


