/* ========================================
 *
 * Copyright NAFU STUDIOS, 2018
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF NAFU STUDIOS.
 *
 * ========================================
*/

/* KNOWN BUGS/TODOS
    1) If you kill a fan before the validation is complete, that command is ignored
    2) Spinning a fan backwards doesn't work all the time (possibly detecting a stop while changing directions?)
    3) Fans sometimes take time to spin up (why validator is needed, possible to fix?)
    4) Add timeout functionality (Counter with ISR, custom count?)
    5) Add LED support for MASTER to indicate status to user
*/

#include "fan.h"
#include "physical.h"
#include "project.h"
#include "rs485.h"

//#define IS_SLAVE
#define IS_MASTER

#if (defined IS_SLAVE && defined IS_MASTER)
#error  Can not define both master and slave   
#endif

// Master Grid for controlling the whole system.
// Each index is a fan where 1 = live and 0 = dead.
uint32_t master_grid[NUM_ROWS][NUM_COLS];

/* 
Each cell contains 4 rows of 8 fans (32 total fans).
Fans are physically indexed like so:
0,  ... , 7
8,  ... , 15
16, ... , 23
24, ... , 31
Fan spinning states are stored in a single uint32_t, one bit per fan
Bit 31 = fan 31, bit 0 = fan 0
1 = fan spinning, 0 = fan stopped
*/

//Functions
#ifdef IS_MASTER
    
// Writes to a singe cell
void master_write_cell(uint32_t cell, uint32_t state) {
    rs485_tx(cell, UART_WRITE, state);
}
    
// Writes all cells to the same state
void master_write_all(uint32_t state) {
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        master_write_cell(i, state);
        CyDelay(1); // slave RX buffer overfills if sent too fast? this will be replaced with a readback eventually
    }
}

// Takes in a 16x16 grid, translates to 8 cell states (8bits each), and sets all states
void master_write_grid(uint32_t grid[NUM_ROWS][NUM_COLS]) {
    // Translate grid into cell states
    uint32_t cell_states[NUM_CELLS];
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

// Reads back single cell state
uint32_t master_read_cell(uint8_t cell) {
    rs485_tx(cell, UART_READ, 0);
    while (UART_GetRxBufferSize() != 6) CyDelayUs(1); // TODO: add timeout
    // Ignore first two bytes for now
    UART_ReadRxData();
    UART_ReadRxData();
    uint32_t read_state = 0;
    read_state  = ((uint32_t) UART_ReadRxData() << 24);
    read_state |= ((uint32_t) UART_ReadRxData() << 16);
    read_state |= ((uint32_t) UART_ReadRxData() <<  8);
    read_state |= ((uint32_t) UART_ReadRxData() <<  0);
    return read_state;   
}

// Updates global master_grid based on cell states
void master_read_grid() {
    for (uint32_t cell = 0; cell < NUM_CELLS; cell++) {
        uint32_t cell_state = master_read_cell(cell);
        for (uint32_t row = 0; row < CELL_ROWS; row++) {
            for (uint32_t col = 0; col < CELL_COLS; col++) {
                uint32_t fan = cell_state & (1 << (row*CELL_COLS + col));
                uint32_t master_row = CELL_ROWS * (cell / 2) + row;
                uint32_t master_col = CELL_COLS * (cell % 2) + col;
                master_grid[master_row][master_col] = fan;
            }
        }
    }
}

#endif // MASTER

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
        
#ifdef IS_MASTER
    // Set UART RX
    uint8_t uart_address = MASTER_ADDRESS;
#endif // MASTER

#ifdef IS_SLAVE
    // Set UART RX Address 
    uint8 uart_address = (Pin_A0_Read() << 0) |
                         (Pin_A1_Read() << 1) |
                         (Pin_A2_Read() << 2);
    // Holds all the states of the fans
    uint32_t ctrl_state = 0;  // last commanded state
    uint32_t curr_state = 0;  // current state
    uint32_t old_state = 0;   // previous state

    // Init fan states
    gpiox_init();
    fan_set_state(0, 1);
#endif // SLAVE

    for(;;)
    {
#ifdef IS_SLAVE           
        // Check if buffer is full
        if (UART_GetRxBufferSize() == UART_RX_BUFFER_SIZE) {
            if (UART_ReadRxData() == UART_READ) {
                // Send back current state if READ
                rs485_tx(uart_address, UART_READ, curr_state);
            } else { 
                // Set control state if WRITE
                ctrl_state  = ((uint32_t) UART_ReadRxData() << 24);
                ctrl_state |= ((uint32_t) UART_ReadRxData() << 16);
                ctrl_state |= ((uint32_t) UART_ReadRxData() <<  8);
                ctrl_state |= ((uint32_t) UART_ReadRxData() <<  0);
            }                        
            // Clear RX buffer after processing data
            UART_ClearRxBuffer();       
        }
        
        // Edit fan state based on user input (curr_state) and master (ctrl_state)
        curr_state = fan_get_state();   // Get current state
        if (curr_state != old_state) {
            // Handles human intervention
            fan_set_state(curr_state, 0); // don't validate since human intervention instantly stops spinning
            ctrl_state = curr_state;
        }
        else if (curr_state != ctrl_state) {
            // Handles commands from master
            fan_set_state(ctrl_state, 1); // validate or spindown will trigger "input"
            curr_state = ctrl_state;
        }
        old_state = curr_state;   

#endif // SLAVE

#ifdef IS_MASTER
        uint32_t state_test;
        for (uint8_t cell = 0; cell < NUM_CELLS; cell++) {
            state_test = 0;
            for (uint8_t fan = 0; fan < FANS_PER_CELL; fan++) {
                state_test |= (1 << fan);
                master_write_cell(cell, state_test);
                while (master_read_cell(cell) != state_test);
                CyDelay(1);   
            }
        }
#endif
    }
}


