#include "daisysp.h"
#include "daisy_pod.h"

// Define namespace for convenence
using namespace daisy;
using namespace daisysp;

// Define hardware
DaisyPod hw;
Oscillator osc;
Svf filt;  // State variable filter 
Adsr env;  // Enveloop generator
