#include "daisysp.h"
#include "daisy_pod.h"

// Define namespace for convenence
using namespace daisy;
using namespace daisysp;

// Define hardware
DaisyPod hw;
Oscillator osc;
Svf filt;  // State variable filter 
Adsr env;  // Envelope generator

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){

     bool gate = hw.button1.Pressed();
    
    for (size_t i = 0; i < size; i++){

        float env_val = env.Process(gate);
        
        // Evaluete the oscillator value
        float osc_val = osc.Process();

        // Filter oscillator output
        filt.Process(osc_val);
        float osc_filt = filt.Low();

        // Combine oscillator and envelope
        float sig = osc_filt * env_val;

        out[0][i] = sig;
        out[1][i] = sig;
    }
}

int main(void){

    // Define sample rate
    float sample_rate = hw.AudioSampleRate();

    // Initialize hardware
    hw.Init();
    hw.StartAdc();

    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SAW)

    filt.Init(sample_rate); // Voltage Controlled Filter
    env.Init(sample_rate); // Envelope generator

    // Envelope parameters
    env.SetAttackTime(0.01f)
    env.SetDecayTime(0.2f)
    env.SetSustainLevel(0.3f)
    env.SetReleaseTime(0.5f)


    hw.ProcessDigitalControls();


}