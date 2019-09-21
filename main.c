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
#include "master.h"
#include "rs485.h"

//#define IS_SLAVE
#define IS_MASTER

// Config error checking
#if (defined IS_SLAVE && defined IS_MASTER)
#error  Can not define both master and slave   
#elif (defined IS_MASTER && UART_RX_HW_ADDRESS1 != 8)
#error  incorrect master UART hardware address
#elif (defined IS_SLAVE && UART_RX_HW_ADDRESS1 >= 8)
#error  incorrect slave UART hardware address
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

    UART_SetRxAddress1(uart_address);  // Doesn't seem to work in hardware address mode
    UART_SetRxAddress2(uart_address);  // Doesn't seem to work in hardware address mode
    UART_Start();

#ifdef IS_MASTER
    //master_write_all(0); // start will a clean state
#endif

    for(;;)
    {
#ifdef IS_SLAVE           
        // Process data if RX buffer full
        if (UART_GetRxBufferSize() == UART_RX_BUFFER_SIZE) {
            uint8 rx_cmd = UART_ReadRxData();
            if (rx_cmd == UART_READ) {
                // Send back current state if READ
                rs485_tx(MASTER_ADDRESS, UART_READ, curr_state);
            } else if (rx_cmd == UART_WRITE) { 
                // Set control state if WRITE
                ctrl_state  = ((uint32_t) UART_ReadRxData() << 24) |
                              ((uint32_t) UART_ReadRxData() << 16) |
                              ((uint32_t) UART_ReadRxData() <<  8) |
                              ((uint32_t) UART_ReadRxData() <<  0);
            }                        
            // Clear RX buffer after processing data
            UART_ClearRxBuffer();       
        }
        
        // Edit fan states based on user input (curr_state) and master (ctrl_state)
        curr_state = fan_get_state();   // Get current state
        if (curr_state != old_state) {
            // Handles human intervention (priority over master)
            fan_set_state(curr_state, 0);  // don't validate since human input has no spindown
            ctrl_state = curr_state;       // make sure ctrl state doesnt take effect next loop
        }
        else if (curr_state != ctrl_state) {
            // Handles commands from master
            fan_set_state(ctrl_state, 1); // validate or spindown will trigger a human input
            curr_state = ctrl_state;
        }
        old_state = curr_state;   

#endif // SLAVE

#ifdef IS_MASTER
    
        uint32_t state_test;
        //for (uint8_t cell = 0; cell < NUM_CELLS; cell++) {
        for (uint8_t cell = 0; cell < NUM_CELLS; cell++) {
            state_test = 0;
            for (uint8_t fan = 0; fan < FANS_PER_CELL; fan++) {
                state_test |= (1 << fan);
                master_write_cell(cell, state_test);
                uint32_t read_back = master_read_cell(cell);
                while (read_back != state_test) {
                    CyDelay(100);
                    read_back = master_read_cell(cell);
                    
                }; //TODO add timeout  
            }
        }
    
#endif
    }
}