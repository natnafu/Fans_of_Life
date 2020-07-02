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

#include "gpiox.h"
#include "physical.h"
#include "project.h"
#include "stopwatch.h"

#define FAN_DETECT_TIME     100 // time to wait for fan edge, units ms

// Reads and combines all status registers into single uint32
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

// Returns the current fan state, blocks for 2 * FAN_DETECT_TIME.
uint32_t fan_get_state(void) {
    // reset status registers
    fan_read_all();
    // Read fans twice to decide if spinning
    CyDelay(FAN_DETECT_TIME);
    uint32_t tmp_state = fan_read_all();
    CyDelay(FAN_DETECT_TIME);
    tmp_state &= fan_read_all();
    return tmp_state;
}

// Starts/stops fans to match `state`.
// Will block wait for `validate_ms` before giving up (timeout).
// If validation is used and fails, the current state is return.
// Otherwise, the set state is returned.
uint32_t fan_set_state(uint32_t state, uint32_t validate_ms) {
    gpiox_send(GPIOXA, ADDR_PORTA, ((uint8_t)(state >> 00)), ((uint8_t)(state >> 8)));
    gpiox_send(GPIOXB, ADDR_PORTA, ((uint8_t)(state >> 16)), ((uint8_t)(state >> 24)));
    
    // Validate new fan state before proceeding
    if (validate_ms) {
        uint32_t val_timer = stopwatch_start();
        uint32_t new_state = fan_get_state();
        while (new_state != state) {
            if (stopwatch_elapsed_ms(val_timer) >= validate_ms) return new_state;
            new_state = fan_get_state();
        }
    }
    return state;
}

// Determines which fans are turning off, starts/resets validation timers, and sets fans to ctrl.
// Handles commands from controller.
uint32_t fan_set_ctrl(uint32_t curr_state, uint32_t ctrl_state, uint32_t validation[FANS_PER_CELL]) {
    uint32_t has_changed = curr_state ^ ctrl_state;     // 1 = change is happening
    uint32_t turned_off = has_changed & curr_state;     // 1 = turned off (has changed and was 1)
    for (uint32_t i = 0; i < FANS_PER_CELL; i++) {
        if (turned_off & (1 << i)) validation[i] = stopwatch_start();   // set validatio time if fan turned off
        else validation[i] = 0;     // else reset any validation
    }
    return fan_set_state(ctrl_state, 0);
}
/* [] END OF FILE */