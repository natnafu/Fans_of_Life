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
#include "stopwatch.h"

// Reads status registers and returns the current state of the fans
uint32_t fan_read_all(void) {
    // Get tach signals from status registers
    uint32_t Reg1 = Status_Reg_1_Read();  // tach 0-7
    uint32_t Reg2 = Status_Reg_2_Read();  // tach 8-15
    uint32_t Reg3 = Status_Reg_3_Read();  // tach 16-23
    uint32_t Reg4 = Status_Reg_4_Read();  // tach 24-31

    // Combine into single uint32_t
    uint32_t tach_signals = (Reg1 | (Reg2 << 8) | (Reg3 << 16)| (Reg4 << 24));
    return tach_signals;
}
 
// Clears status registers, wait for them to reset, then return current fan state
uint32_t fan_get_state() {
    fan_read_all(); // reset status registers
    CyDelay(100);   // allow a 10Hz spin to register
    return fan_read_all();
}

// Starts/stops fans to match `state`.
// Will block wait for `validate_ms` before giving up (timeout).
void fan_set_state(uint32_t state, uint32_t validate_ms) {
    gpiox_send(GPIOXA, ADDR_PORTA, ((uint8_t)(state >> 00)), ((uint8_t)(state >> 8)));
    gpiox_send(GPIOXB, ADDR_PORTA, ((uint8_t)(state >> 16)), ((uint8_t)(state >> 24)));
    
    // Validate new fan state before proceeding
    if (validate_ms) {
        uint32_t val_timer = stopwatch_start();
        uint32_t new_state = fan_get_state();
        while (new_state != state) {
            if (stopwatch_elapsed_ms(val_timer) >= validate_ms) break;
            new_state = fan_get_state();
        }
    }
}
/* [] END OF FILE */