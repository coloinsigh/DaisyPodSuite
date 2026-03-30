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

    // Filter cutoff from knob1, scaled between [20, 10k] Hz
    float cutoff = 20.0f +  (hw.knob1.Process() * 9980.0f)
    filt.SetFreq(cutoff)

    // Resosnance from knob 2, in range [0.0, 1.0]
    float res = hw.knob2.Process()
    filt.SetRes(res)
    
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
    // Initialize hardware
    hw.Init();

    // Define sample rate
    float sample_rate = hw.AudioSampleRate();

    hw.StartAdc();

    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SAW);

    filt.Init(sample_rate); // Voltage Controlled Filter
    env.Init(sample_rate); // Envelope generator

    // Envelope parameters
    env.SetAttackTime(0.01f);
    env.SetDecayTime(0.2f);
    env.SetSustainLevel(0.3f);
    env.SetReleaseTime(0.5f);



    hw.StartAudio(AudioCallback);

    while(1) {
        hw.ProcessDigitalControls();

        // Set cutoff of filter
        float cutoff = 20.0f + (hw.knob1.Process() * 19980.0f);
        filt.SetFreq(cutoff);
        
    }

}