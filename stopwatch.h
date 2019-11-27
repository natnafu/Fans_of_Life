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
#pragma once
#include "project.h"

// Returns the current timer count
uint32_t stopwatch_start(void) {
    return Timer_ReadCounter();
}

// Returns the difference in ms between the given and current count (assumes 1kHz count clock)
uint32_t stopwatch_elapsed_ms(uint32_t time_ms) {
    uint32_t curr_time_ms = Timer_ReadCounter();
    if (time_ms > curr_time_ms) return (time_ms - curr_time_ms);
    // Handle counter rollover
    // Subtraction has to happen before the addition to prevent another rollover
    //uint32_t rollover_time_ms = Timer_ReadPeriod() - curr_time_ms;
    return (time_ms + (Timer_ReadPeriod() - curr_time_ms));
}

/* [] END OF FILE */
