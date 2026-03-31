#include "daisysp.h"
#include "daisy_pod.h"

// Define namespace for convenence
using namespace daisy;
using namespace daisysp;

// Define hardware
DaisyPod hw;
Oscillator osc;
Oscillator osc_sub;
Svf filt;  // State variable filter 
Adsr env;  // Envelope generator

// Define scales and indexes for keeping track of notes
float scales[2][8] = {
    {60, 62, 64, 65, 67, 69, 71, 72},  // C major
    {60, 62, 63, 65, 67, 68, 70, 72},  // C minor
};

int current_scale = 0;
int note_index = 0;
bool sub_enabled = true;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){

    bool gate = hw.button1.Pressed();
    
    for (size_t i = 0; i < size; i++){

        float env_val = env.Process(gate);
        
        // Evaluete the oscillator value
        float osc_val = osc.Process();

        float osc_mixed;
        if (sub_enabled){
            float osc_sub_val = osc_sub.Process();
            osc_mixed = (osc_val + osc_sub_val) * 0.5f;
        } else {
            osc_mixed = osc_val;
        }

        // Filter oscillator output
        filt.Process(osc_mixed);
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

    // Define soprano oscillator
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    osc.SetFreq(440.0f);

    // Define bass oscillator
    osc_sub.Init(sample_rate);
    osc_sub.SetWaveform(Oscillator::WAVE_SQUARE);
    osc_sub.SetAmp(0.5f);
    osc_sub.SetFreq(440.0f / 2.0f);

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

        // Find encoder increments
        int inc = hw.encoder.Increment();
        note_index += inc;
        note_index = std::clamp(note_index, 0, 7);

        // Check for scale change with button 2
        if (hw.button2.RisingEdge()){
            current_scale = !current_scale;
        }

        // Poll encoder click to enable/disable sub
        if (hw.encoder.RisingEdge()){
            sub_enabled = !sub_enabled;
        }

        float midi_note = scales[current_scale][note_index];

        // Set LED status based on current scale
        if (current_scale){
            hw.led1.Set(1.0f, 1.0f, 1.0f);
            hw.led2.Set(0.0f, 0.0f, 0.0f);
        } else {
            hw.led1.Set(0.0f, 0.0f, 0.0f);
            hw.led2.Set(1.0f, 1.0f, 1.0f);
        }
        hw.UpdateLeds();

        osc.SetFreq(mtof(midi_note));
        osc_sub.SetFreq(mtof(midi_note) / 2.0f);

        // Filter cutoff from knob1, scaled between [20, 10k] Hz
        float cutoff = 20.0f +  (hw.knob1.Process() * 9980.0f);
        filt.SetFreq(cutoff);

        // Resosnance from knob 2, in range [0.0, 1.0]
        float res = hw.knob2.Process();
        filt.SetRes(res);
        
        System::Delay(1); // Small delay to prevent overwhelming board
    }

}