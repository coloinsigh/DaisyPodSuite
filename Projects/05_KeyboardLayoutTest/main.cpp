#include "daisysp.h"
#include "daisy_pod.h"
#include <algorithm>

// Define the namespace for convenience
using namespace daisy;
using namespace daisysp;

constexpr int NUM_VOICES = 8;

// Initialize objects
DaisyPod hw;
I2CHandle i2c;

struct Voice {
    Oscillator osc;
    AdEnv env;
    bool is_active;
};
Voice voice_pool[NUM_VOICES];

float target_freq = 440.0;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
    for (size_t i = 0; i < size; i++){

        for (int x = 0; x < NUM_VOICES; x++){
            voice_pool[x].osc.SetFreq(target_freq);

            float synth_signal = voice_pool[x].osc.Process() * voice_pool[x].env.Process();
        
            out[0][i] = synth_signal;
            out[1][i] = synth_signal;
        }
    }
}


int main(void) {
    hw.Init();


    // Initializae oscillator
    float sample_rate = hw.AudioSampleRate();
    osc.Init(sample_rate);
}