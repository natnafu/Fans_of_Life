/* ========================================
 *
 * Copyright NAFU STUDIOS, 2019
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF NAFU STUDIOS.
 *
 * ========================================
*/

#include "project.h"
#include "gpiox.h"
#include "physical.h"

/*
 - phsyical fan layout doesn't match the tach channels they are plugged into.
 - This array translates the tach signal to fan index.
 - index = fan, value = tach signal.
*/
uint8_t tach_translation[FANS_PER_CELL] = {
    16, 17, 18, 19, 20, 21, 22, 23,
    15, 14, 13, 12, 27, 26, 25, 24,
     8,  9, 10, 11, 28, 29, 30, 31,
     7,  6,  5,  4,  3,  2,  1,  0
};

// Reads status registers and returns the current state of the fans
uint32_t fan_read_all(void) {
    // Get tach signals from status registers
    uint32_t Reg1 = Status_Reg_1_Read();  // tach 0-7
    uint32_t Reg2 = Status_Reg_2_Read();  // tach 8-15
    uint32_t Reg3 = Status_Reg_3_Read();  // tach 16-23
    uint32_t Reg4 = Status_Reg_4_Read();  // tach 24-31

    // Combine into single uint32_t
    uint32_t tach_signals = (Reg1 | (Reg2 << 8) | (Reg3 << 16)| (Reg4 << 24));
    
    // Translate tach signals to correct fan
    uint32_t fan_signals = 0;
    for (uint32_t i = 0; i < FANS_PER_CELL; i++) {
        if (tach_signals & (1 << tach_translation[i])) {
            fan_signals |= (1 << i) ;               
        }
    }
    return fan_signals;
}
 
// Clears status registers, wait for them to reset, then return current fan state
#define DEBOUNCE 1 // not actually a debounce...
uint32_t fan_get_state() {
    uint32_t count = 0;
    for (uint32_t i = 0; i < DEBOUNCE; i++) {
        fan_read_all(); // reset status registers
        CyDelay(100 / DEBOUNCE);   // wait for tachs to trigger
    }
    return fan_read_all();
}

// Starts/stops fans to match `state`
// `validate` can be used if system should pause until actual state matches set state
void fan_set_state(uint32_t state, uint8_t validate) {
    
    // Translate tach signals to correct fan
    uint32_t trans_state = 0;
    for (uint32_t i = 0; i < FANS_PER_CELL; i++) {
        if (state & (1 << i)) {
            trans_state |= (1 << tach_translation[i]);               
        }
    }
    
    //TODO: make a control translation array to get rid of this bit shifting
    uint32_t sh_state = (trans_state << 4) | (trans_state >> 28); // BUGFIX: schematic has control pins off by 4, bit shift w/ wrap around
    gpiox_send(GPIOXA, ADDR_PORTA, ((uint8_t)(sh_state >> 00)), ((uint8_t)(sh_state >> 8)));
    gpiox_send(GPIOXB, ADDR_PORTA, ((uint8_t)(sh_state >> 16)), ((uint8_t)(sh_state >> 24)));
    
    // Validate new fan state before proceeding
    // TODO: Add timeout
    if (validate) {
        uint32_t new_state = fan_get_state(); 
        while (new_state != state) {
            new_state = fan_get_state();    
        }
    }
}
/* [] END OF FILE */