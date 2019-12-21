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
#include "physical.h"
#include "project.h"
#include "rs485.h"
#include "stopwatch.h"

// Writes to a singe cell, blocks until response received
void master_write_cell(uint32_t cell, uint32_t state) {
    rs485_tx(cell, UART_WRITE, state);
    uint32_t timer_confirmation = stopwatch_start();
    while (UART_GetRxBufferSize() != UART_RX_BUFFER_SIZE) {
        if (stopwatch_elapsed_ms(timer_confirmation) >= TOUT_SLV_RSP) break;   // timeout
    }
    UART_ClearRxBuffer();
    CyDelay(10);    // delay needed for all to rsp to command. Likely a bus confliction issue, deal with later
}
   
// Reads back single cell state
uint32_t master_read_cell(uint8_t cell) {
    rs485_tx(cell, UART_READ, 0);
    uint32_t timer_confirmation = stopwatch_start();
    uint32_t elapsed_time = 0;
    while (UART_GetRxBufferSize() != UART_RX_BUFFER_SIZE) {
        CyDelay(1);
        elapsed_time = stopwatch_elapsed_ms(timer_confirmation);
        if (elapsed_time >= TOUT_SLV_RSP) {
        //if (stopwatch_elapsed_ms(timer_confirmation) >= TOUT_SLV_RSP) {
            // Timeout, clear buffer and send back 0s
            UART_ClearRxBuffer();
            return 0;
        }
    }
    UART_ReadRxData();  // Ignore first R/W byte for now
    uint32_t response = ((uint32_t) UART_ReadRxData() << 24) |
                        ((uint32_t) UART_ReadRxData() << 16) |
                        ((uint32_t) UART_ReadRxData() <<  8) |
                        ((uint32_t) UART_ReadRxData() <<  0);
    //UART_ClearRxBuffer();
    CyDelay(10);    // delay needed for all to rsp to command. Likely a bus confliction issue, deal with later
    return response;
}

// Writes all cells to the same state
void master_write_all(uint32_t state) {
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        master_write_cell(i, state);
    }
}

// Takes in a 16x16 grid, translates to 8 cell states (8bits each), and sets all states
void master_write_grid(uint32_t grid[NUM_ROWS][NUM_COLS]) {
    // Translate grid into cell states
    uint32_t cell_states[NUM_CELLS] = {0};
    for (uint32_t row = 0; row < NUM_ROWS; row++) {     // use # of MASTER rows
        for (uint32_t col = 0; col < NUM_COLS; col++) { // use # of MASTER cols
            if (grid[row][col]) {
                uint32_t cell = (col / CELL_COLS) + 2 * (row/CELL_ROWS);
                cell_states[cell] |= (1 << ((row % CELL_ROWS)* CELL_COLS + (col % CELL_COLS)));
            }
        }
    }
    for (uint32_t cell = 0; cell < NUM_CELLS; cell++) {
        master_write_cell(cell, cell_states[cell]);
    }
}

// Updates grid based on cell states
void master_read_grid(uint32_t grid[NUM_ROWS][NUM_COLS]) {
    for (uint32_t cell = 0; cell < NUM_CELLS; cell++) {
        uint32_t cell_state = master_read_cell(cell);
        for (uint32_t row = 0; row < CELL_ROWS; row++) {
            for (uint32_t col = 0; col < CELL_COLS; col++) {
                uint32_t fan = cell_state & (1 << (row*CELL_COLS + col));
                uint32_t master_row = CELL_ROWS * (cell / 2) + row;
                uint32_t master_col = CELL_COLS * (cell % 2) + col;
                grid[master_row][master_col] = fan;
            }
        }
    }
}

/* [] END OF FILE */
