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
    

## Build Instructions
1. Clone repo
2. Enter project folder
    - Enter boot mode on Daisy Pod (press and hold boot button > press reset button > release boot button )
3. Build: `make`
4. Flash: `make program-dfu`
