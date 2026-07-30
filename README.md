# Polyphonic Synthesizer Engine & Hardware Prototyping

A low-latency, multi-voice polyphonic digital synthesizer engine built on the **Daisy Seed** (STM32H750) embedded hardware platform. 

This repository serves as the core embedded framework and physical prototyping testbed for a future custom hardware instrument. It focuses on testing physical button matrix layouts and spacing, evaluating hardware I/O expansion via I2C, and optimizing real-time audio callback performance.

---

# Key Technical Highlights

* **Multi-Voice Polyphonic Synthesis Engine:** Built in C++ using `libDaisy` and `DaisySP`, generating polyphonic triangle/saw wave synthesis with dedicated ADSR envelope generators per voice. This has been prototyped with a custom PCB with 8 voices, with ability for expansion.
* **Non-Blocking I2C Hardware Polling:** Interfaces with an **MCP23017** 16-bit GPIO expander operating in Fast-Mode (400kHz) to read physical keybed matrix inputs without blocking the audio processing thread.
* **Non-Blocking Software Debouncing:** Implements a time-window state filter per voice to eliminate physical contact bounce chatter on both key press and key release without introducing playable audio latency.
* **Thread-Safe Architecture:** Strict separation between high-priority real-time audio callbacks (audio interrupt thread) and background hardware polling loops (main thread).
* **Headroom & Saturation Management:** Dynamic voice amplitude scaling and output clamping to prevent digital DAC clipping during dense polyphonic chords.

---

# Repository Structure
```text
.
├── README.md                <-- Project overview and specifications
├── .gitignore               <-- Filters out build binaries (.bin, .elf, .o)
├── docs/
│   ├── hardware-spec.md     <-- Circuit diagrams, pinout tables, and I2C specs
│   └── lessons-learned.md   <-- Deep dive on switch debouncing & I2C hardware debugging
├── firmware/                <-- Production Synthesizer & I2C Firmware (C++)
│   ├── main.cpp
│   └── Makefile
└── examples/                <-- Step-by-step modular development history
    ├── 01_SawOscillator/
    ├── 02_SubtractiveSynth/
    ├── 03_FX_Pedal/
    └── 04_ButtonInput/
```

---

## Examples
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
- **05_KeyboardLayoutTest**
    - Digital hardware synth with two 8 button keyboards
    - Keyboards are intended as a prototype for specific layouts
    - Each keyboard is addressed through separate addresses on I2C; GPIOA or GPIOB
    - Polyphony is supported but the same note played by both keyboards is intentionally disabled
    - Non-blocking button debouncing; avoids ghost triggers and multiple notes being played. Debounce threshold is > 5ms

---    

## Build Instructions
1. Clone repo
2. Navigate to production firmware
    - `cd firmware`
3. Compile project:
    - `make`
4. Enter boot mode on Daisy Pod (press and hold boot button > press reset button > release boot button )
5. Flash to the Daisy Seed vis USB DFU: `make program-dfu`

---

## Useful extras
Where appropriate, `hw.seed.PrintLine` commands are used for serial monitoring over USB. When used, a useful way to view the serial monitor is with `screen` on Ubuntu or macOS. On Windows, PuTTY is a good alternative.