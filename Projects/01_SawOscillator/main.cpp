#include "daisysp.h"
#include "daisy_pod.h"
#include <algorithm>

// Define the namespace for convenience
using namespace daisy;
using namespace daisysp;

// Define the hardware, oscillator and noise objects
DaisyPod hw;
Oscillator osc;
WhiteNoise noise;
Svf low_pass;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    
    float knob1_value = hw.knob1.Process(); // Read the value of knob 1 (0.0 to 1.0)
    float knob2_value = hw.knob2.Process(); // Read the value of knob 2 (0.0 to 1.0)
    float cutoff = 20.0f + (knob2_value * 22000.0f); // Map knob 2 to a cutoff frequency between 20Hz and 22kHz
    low_pass.SetFreq(cutoff); // Set the cutoff frequency of the low-pass filter
    
    for (size_t i = 0; i < size; i++) {
        float val = osc.Process(); // Get the next sample from the oscillator
        float noise_val = noise.Process() * knob1_value; // Scale noise by the knob value

        float mixed_val = (val  * 0.6f) + (noise_val * 0.4f); // Mix the oscillator and noise

        // Run the mixed signal through the low-pass filter
        low_pass.Process(mixed_val);

        float filtered_val = low_pass.Low(); // Get the low-pass filtered output

        out[0][i] = filtered_val; // Output to left channel
        out[1][i] = filtered_val; // Output to right channel
    }
}

int main(void) {
    hw.Init();
    hw.StartAdc();
    noise.Init();


    // Define the sample rate
    float sample_rate = hw.AudioSampleRate();

    // LPF initialization
    low_pass.Init(sample_rate);
    low_pass.SetRes(0.7f); // Set resonance (Q factor)
    low_pass.SetDrive(0.0f); // Set drive amount

    // Initialize the oscillator with the sample rate
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SAW); // Set waveform to sine
    osc.SetFreq(440.0f); // Set frequency to A4

    hw.StartAudio(AudioCallback);

    float current_freq = 440.0f;
    uint32_t reset_time = 0; // Variable to track the time of the last reset

    float max_freq = 2800.0f;
    float min_freq = 50.0f;

    while(1) {
        hw.ProcessDigitalControls();
        Color colour; 

        current_freq += (10.0f * hw.encoder.Increment());
        current_freq = std::clamp(current_freq, min_freq, max_freq); // Limit frequency to a reasonable range
        osc.SetFreq(current_freq);

        // If encoder is clicked, reset to 440Hz
        if (hw.encoder.RisingEdge()) {
            current_freq = 440.0f;
            reset_time = System::GetNow(); // Update reset time to current time
        }

        // If button 1 is pressed, drop by an octave
        if (hw.button1.RisingEdge()) {
            current_freq /= 2.0f; // Drop frequency by an octave
            current_freq = std::clamp(current_freq, min_freq, max_freq); // Ensure it doesn't go below minimum
            osc.SetFreq(current_freq);
        }

        // If button 2 is pressed, raise by an octave
        if (hw.button2.RisingEdge()) {
            current_freq *= 2.0f; // Raise frequency by an octave
            current_freq = std::clamp(current_freq, min_freq, max_freq); // Ensure it doesn't go above maximum
            osc.SetFreq(current_freq);
        }  

        if (System::GetNow() - reset_time < 500) { // Check if within 500ms of the last reset
            colour.Init(1.0f,1.0f,1.0f); // Set to white
            hw.led1.SetColor(colour);
            hw.UpdateLeds();
            }
        else {
            // Normalize frequency between the acceptable range
            float normalized_freq = (current_freq - min_freq) / (max_freq - min_freq);

            colour.Init(normalized_freq, 1.0f - normalized_freq, 0.0f); // Set red based on frequency
            
            hw.led1.SetColor(colour);
            hw.UpdateLeds();
        }

        System::Delay(1); // include a small delay to prevent overwhelming the CPU
    
    }
}