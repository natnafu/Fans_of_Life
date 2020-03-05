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

// timeouts, units of ms
#define TOUT_FAN_SET    6000    // Fan state validation timeout (fans take ~6s to spin down)
#define TOUT_RX_COMM    300     // UART RX timeout from first byte (packet should take 0.5ms to finish)
#define TOUT_SLV_RSP    500     // Slave responce timeout.

// Returns the current timer count
uint32_t stopwatch_start(void) {
    return Timer_ReadCounter();
}

// Returns the difference in ms between the given and current count (assumes 1kHz count clock)
uint32_t stopwatch_elapsed_ms(uint32_t time_ms) {
    uint32_t curr_time_ms = Timer_ReadCounter();
    if (time_ms >= curr_time_ms) return (time_ms - curr_time_ms);
    // Handle counter rollover
    return (time_ms + (Timer_ReadPeriod() - curr_time_ms));
}

/* [] END OF FILE */
