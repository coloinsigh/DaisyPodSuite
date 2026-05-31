# Experiments with the DaisyPod

A collection of custom firmare to explore aspects of the DaisyPod.

## Projects
- **01_SawOcillator**: A sawtooth oscillator featuring:
    - Frequency control through the Daisy Pod main encoder
    - White noise mixing through Potentiometer 1
    - Low Pass Filtering through Potentiometer 2
    - Octave Shifting through buttons 1 & 2
    - Click to centre oscillator on 440 Hz (encoder)
- **02_SubtractiveSynth**
    - ADSR style subtractive synthesis - East Coast style
    - State variable filter to add low pass filtering
- **04_ButtonInput**
    - Add support for external button input
    - Three buttons incorporated on pins A1, A2, A3 of a HW-839 breakout board
    - Monophonic synth engine tuned to a single triad as a MVP
    - Barebones envelope
    

## Build Instructions
1. Clone repo
2. Enter project folder
    - Enter boot mode on Daisy Pod (press and hold boot button > press reset button > release boot button )
3. Build: `make`
4. Flash: `make program-dfu`


## Useful extras
Where appropriate, `hw.seed.PrintLine` commands are used for serial monitoring over USB. When used, a useful way to view the serial monitor is with `screen` on Ubuntu or macOS. On Windows, PuTTY is a good alternative. 