#include "daisysp.h"
#include "daisy_pod.h"
#include <algorithm>

// Define the namespace for convenience
using namespace daisy;
using namespace daisysp;

// Define the hardware interface
DaisyPod hw;

// Create the I2C object globally
I2CHandle i2c_bus;

// Define oscillator
Oscillator osc;
AdEnv env;

float target_freq = 261.63f;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
    for (size_t i = 0; i < size; i++){
        osc.SetFreq(target_freq);

        float synth_signal = osc.Process() * env.Process();

        out[0][i] = synth_signal;
        out[1][i] = synth_signal;
    }
}


int main(void) {
    hw.Init();
    hw.seed.StartLog(true); // Start logger

    // Initialize oscillator
    float sample_rate = hw.AudioSampleRate();
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_TRI); 
    osc.SetFreq(target_freq);
    osc.SetAmp(0.5f);

    // Volume envelope
    env.Init(sample_rate);
    env.SetTime(ADENV_SEG_ATTACK, 0.05f);   // attack
    env.SetTime(ADENV_SEG_DECAY, 0.4f);     // decay
    env.SetMax(1.0f);                       // max volume

    // Start Audio
    hw.StartAudio(AudioCallback);

    // 1. Create a configuration structure for I2C
    I2CHandle::Config i2c_config;
    
    // 2. Choose the first I2C peripheral on the chip and set standard fast speed
    i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1; 
    i2c_config.speed  = I2CHandle::Config::Speed::I2C_400KHZ;
    
    i2c_config.mode   = I2CHandle::Config::Mode::I2C_MASTER;

    i2c_config.pin_config.scl = seed::D11;
    i2c_config.pin_config.sda = seed::D12;

    i2c_bus.Init(i2c_config);

    hw.seed.PrintLine("Initialized I2C bus");

    // Handshake
    uint8_t buffer[2];
    buffer[0] = 0x00;
    buffer[1] = 0xFF;

    hw.seed.PrintLine("Logging 1");

    I2CHandle::Result result = i2c_bus.TransmitBlocking(0x20, buffer, 2, 10);

    hw.seed.PrintLine("Logging 2");

    if (result == I2CHandle::Result::OK) {
        hw.seed.PrintLine("True");
    }
    else {
        hw.seed.PrintLine("False");
    }

    hw.seed.PrintLine("Logging 3");

    // Pull up 
    uint8_t pullup_buffer[2];
    pullup_buffer[0] = 0x0C; // Target the Pull-up register (GPPUA)
    pullup_buffer[1] = 0xFF; // Enable pull-ups on all Port A pins

    I2CHandle::Result pu_result = i2c_bus.TransmitBlocking(0x20, pullup_buffer, 2, 10);

    if (pu_result == I2CHandle::Result::OK) {
        hw.seed.PrintLine("Pull-up resistors active!");
    } else {
        hw.seed.PrintLine("Failed to activate pull-ups.");
    }

    while(1) {
        hw.ProcessDigitalControls();

        uint16_t reg_to_read = 0x12; // GPIOA register to read
        uint8_t port_state = 0xFF; // Variable to hold the byte returned

        // Send 0x12, then read 1 byte back into port_state
        I2CHandle::Result read_result = i2c_bus.ReadDataAtAddress(
            0x20,           // Device adress
            reg_to_read,    // register to read
            1,              // size of register
            &port_state,    // where to store result
            1,              // number of bytes to read
            10              // timeout in ms
        );

        if (read_result == I2CHandle::Result::OK) {
            // port_state now holds current state of Port A
            // 1 Floating
            // 0 Grounded

            bool btn1_pressed = ((port_state & 0x01) == 0);
            bool btn2_pressed = ((port_state & 0x02) == 0);
            bool btn3_pressed = ((port_state & 0x04) == 0);

            if (btn1_pressed) {
                target_freq = 261.63f;
                env.Trigger();
            }
            if (btn2_pressed) {
                target_freq = 329.63f;
                env.Trigger();
            }
            if (btn3_pressed) {
                target_freq = 392.00f;
                env.Trigger();
            }

            // Print out state of each button
            // hw.seed.PrintLine("B1: %d | B2: %d | B3: %d", (int)btn1_pressed, (int)btn2_pressed, (int)btn3_pressed);
        }
    
        System::Delay(10);
    }
}