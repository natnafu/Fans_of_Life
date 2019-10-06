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

uint8_t volatile rx_data_ready = 0;

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */  

#ifdef IS_SLAVE
    // Holds all the states of the fans
    uint32_t ctrl_state = 0;  // commanded state
    uint32_t old_state = 0;   // previous state
    uint32_t curr_state = 0;  // current state

    // Init fan states
    gpiox_init();
    fan_set_state(0, 0); // NAT set to validate
    //curr_state = fan_read_all();
#endif // SLAVE

    UART_Start();

    for(;;)
    {
#ifdef IS_SLAVE           
        // Grab current state
        curr_state = fan_get_state();
        
        // Check for human intervention
        if (curr_state != old_state) {
            fan_set_state(curr_state, 0);  // don't validate since human input has no spindown
        }
        
        // Check for commands
        if (UART_GetRxBufferSize() == UART_RX_BUFFER_SIZE) {
            uint8 rx_cmd = UART_ReadRxData();
            if (rx_cmd == UART_READ) {
                rs485_tx(MASTER_ADDRESS, UART_READ, curr_state);
            } else if (rx_cmd == UART_WRITE) { 
                ctrl_state  = ((uint32_t) UART_ReadRxData() << 24) |
                              ((uint32_t) UART_ReadRxData() << 16) |
                              ((uint32_t) UART_ReadRxData() <<  8) |
                              ((uint32_t) UART_ReadRxData() <<  0);
                fan_set_state(ctrl_state, 0); // NAT: change this to validating
                curr_state = ctrl_state;
                rs485_tx(MASTER_ADDRESS, UART_WRITE, curr_state);
            }                        
            // Clear RX buffer after processing data
            UART_ClearRxBuffer();       
        }
       
        old_state = curr_state;   
#endif // SLAVE

#ifdef IS_MASTER
        master_write_cell(0, 0);
        CyDelay(10000);
        uint32_t state_test;
        //for (uint8_t cell = 0; cell < NUM_CELLS; cell++) {
        for (uint8_t cell = 0; cell < 1; cell++) {
            state_test = 0;
            for (uint8_t fan = 0; fan < FANS_PER_CELL; fan++) {
                state_test |= (1 << fan);
                master_write_cell(cell, state_test);
                CyDelay(6000);
                //uint32_t read_back = master_read_cell(cell);
                //while (read_back != state_test) {
                    //TODO add timeout
                //}   
            }
        }  
#endif
    }
}