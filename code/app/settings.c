#include "settings.h"

// Default settings
settings_t settings = {
    8500, // knockFreq
    3000, // knockRatio
    500,  // tpsMinV
    4500, // tpsMaxV
    3,    // fuelMinTh
    3,    // fuelMaxChange
    70,   // AfrMinVal*10
    220,  // AfrMaxVal*10
    0,    // AfrOffset
    0,    // sensorsInput
    0,    // afrInput
    0,    // reserved1
    0,    // reserved2
    0     // reserved3
};
