#include "daisysp.h"
#include "daisy_pod.h"
#include <algorithm>

// Define the namespace for convenience
using namespace daisy;
using namespace daisysp;

constexpr int NUM_VOICES = 8;
constexpr uint8_t DEBOUNCE_DELAY_MS = 5; 

// Initialize objects
DaisyPod hw;
I2CHandle i2c_bus;

struct Voice {
    Oscillator osc;
    AdEnv env;
    bool is_active;
};
Voice voice_pool[NUM_VOICES];

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
    float synth_signal_global = 0;
    for (size_t i = 0; i < size; i++){
        
        // Reset global synth output at every iteration
        synth_signal_global = 0;
        for (int x = 0; x < NUM_VOICES; x++){
            // Combine each voice into the global output
            float synth_signal = voice_pool[x].osc.Process() * voice_pool[x].env.Process();
            synth_signal_global += synth_signal;
        }

        // Ensure output signal never oversaturates
        synth_signal_global = std::clamp(synth_signal_global, -1.0f, 1.0f);

        out[0][i] = synth_signal_global;
        out[1][i] = synth_signal_global;
    }
}


int main(void) {
    hw.Init();

    // Initialize an array to hold the status of button press per voice, along with debouncing arrays
    bool note_pressed[NUM_VOICES] = {false};
    uint32_t last_bounce_time[NUM_VOICES] = {0};
    bool current_state[NUM_VOICES] = {false};
 

    // Initializae oscillator
    float sample_rate = hw.AudioSampleRate();
    for (int x = 0; x < NUM_VOICES; x++){
        // Initialize Oscillator for each button
        voice_pool[x].osc.Init(sample_rate);
        voice_pool[x].osc.SetWaveform(Oscillator::WAVE_TRI);
        voice_pool[x].osc.SetFreq(SCALE_LUT[x]);
        voice_pool[x].osc.SetAmp(0.15f);    // Reduce chance of saturation

        // Initialize envelope for each button
        voice_pool[x].env.Init(sample_rate);
        voice_pool[x].env.SetTime(ADENV_SEG_ATTACK, 0.05f); // attack
        voice_pool[x].env.SetTime(ADENV_SEG_DECAY, 0.4f);   // decay
        voice_pool[x].env.SetMax(1.0f);                     // max volume
    }

    // Handle I2C peripherals for polling button press
    I2CHandle::Config i2c_config;
    i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1; 
    i2c_config.speed  = I2CHandle::Config::Speed::I2C_400KHZ;
    
    i2c_config.mode   = I2CHandle::Config::Mode::I2C_MASTER;

    i2c_config.pin_config.scl = seed::D11;
    i2c_config.pin_config.sda = seed::D12;
    
    i2c_bus.Init(i2c_config);

    // Set up internal registers for I2C interaction
    uint8_t mcp_addr = 0x20;
    uint8_t tx_data[2];
    uint8_t registers[4] = {0x00, 0x01, 0x0C, 0x0D}; 

    for (int i = 0; i < 4; i++) {
        tx_data[0] = registers[i];
        tx_data[1] = 0xFF;
        i2c_bus.TransmitBlocking(mcp_addr, tx_data, 2, 10);
    }

    // Start audio
    hw.StartAudio(AudioCallback);

    while(1){
        hw.ProcessDigitalControls();

        // Handle button pressing logic
        uint16_t reg_to_read = 0x12;  // GPIOA
        uint8_t port_data[2] = {0xFF, 0xFF};  // Initialize port outputs

        // Utilitze DaisySeed's automatic address pointer incrementing - start reading from
        // 0x12 and read 2 bytes 
        I2CHandle::Result read_result = i2c_bus.ReadDataAtAddress(
            0x20,           // Device addres
            reg_to_read,    // register to read
            1,              // size of register
            port_data,      // result storage
            2,              // number of bytes to read
            10              // timeout in ms
        );

        // Now parse data into useful variables
        uint8_t port_a = port_data[0];
        uint8_t port_b = port_data[1];

        // Debouncing prevents multiple key presses registering for a single button press
        // Non-blocking debouncing is implemented below, where additional key presses aren't 
        // blocked by listening and checking for note status.
        // Get current time for debouncing and initialize variables
        uint32_t now = System::GetNow();
        
        for (int x=0; x < NUM_VOICES; x++){
            bool keys_a_pressed = ((port_a >> x) & 1) == 0;
            bool keys_b_pressed = ((port_b >> x) & 1) == 0;

            // Prevent both keyboards from playing the same note simultaneously while handling debouncing
            bool note_is_active = keys_a_pressed || keys_b_pressed;

            if ((note_is_active != current_state[x]) && (now - last_bounce_time[x] > DEBOUNCE_DELAY_MS)) {
                current_state[x] = note_is_active;
                last_bounce_time[x] = now;
            }

            // Check if a button is newly pressed
            if (current_state[x] && !note_pressed[x]){
                // Trigger note
                voice_pool[x].env.Trigger();
                note_pressed[x] = true;
            }
            else if (!current_state[x] && note_pressed[x]){
                // Note has been released - trust AdEnv::Trigger() to handle fade out
                note_pressed[x] = false;
            }
        }
    }
}