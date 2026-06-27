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

// Define a LUT containing 8 notes in C
const float SCALE_LUT[NUM_VOICES] = {
    261.63f, // C4
    293.66f, // D4
    329.63f, // E4
    349.23f, // F4
    392.00f, // G4
    440.00f, // A4
    493.88f, // B4
    523.25f  // C5
};


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
    for (int x = 0; x < NUM_VOICES; x++){
        // Initialize Oscillator for each button
        voice_pool[x].osc.Init(sample_rate);
        voice_pool[x].osc.SetWaveform(Oscillator::WAVE_TRI);
        voice_pool[x].osc.SetFreq(SCALE_LUT[x]);

        // Initialize envelope for each button
        voice_pool[x].env.Init(sample_rate);
        voice_pool[x].env.SetTime(ADENV_SEG_ATTACK, 0.05f); // attack
        voice_pool[x].env.SetTime(ADENV_SEG_DECAY, 0.4f);   // decay
        voice_pool[x].env.SetMax(1.0f);                     // max volume
    }
}