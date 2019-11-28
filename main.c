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
    2) Spinning a fan backwards doesn't work all the time (possibly detecting a stop while changing directions)
    5) Add LED support for MASTER to indicate status to user
*/

#include "fan.h"
#include "physical.h"
#include "project.h"
#include "master.h"
#include "rs485.h"
#include "stopwatch.h"

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

int main(void)
{
    CyGlobalIntEnable;      // Enable global interrupts.
    Timer_Start();          // Used for stopwatch timers
    UART_Start();           // Used for RS485 comms
    uint32_t timer_comm;    // RS485 communication timer
    uint8_t rx_buff_size;   // Stores UART RX buffer size

#ifdef IS_SLAVE
    // Holds all the states of the fans
    uint32_t ctrl_state = 0;  // commanded state
    uint32_t old_state  = 0;  // previous state
    uint32_t curr_state = 0;  // current state

    // Init fan states to 0
    gpiox_init();
    fan_set_state(0, TOUT_FAN_SET);
#endif // SLAVE

    for(;;)
    {
#ifdef IS_SLAVE
        // Grab current state
        curr_state = fan_get_state();
        
        // Check for human intervention
//        if (curr_state != old_state) {
//            fan_set_state(curr_state, 0);  // don't validate since human input has no spindown
//        }
        
        // Check for commands
        rx_buff_size = UART_GetRxBufferSize();
        if (rx_buff_size == UART_RX_BUFFER_SIZE) {
            uint8 rx_cmd = UART_ReadRxData();
            if (rx_cmd == UART_READ) {
                rs485_tx(MASTER_ADDRESS, UART_READ, curr_state);
            } else if (rx_cmd == UART_WRITE) { 
                ctrl_state  = ((uint32_t) UART_ReadRxData() << 24) |
                              ((uint32_t) UART_ReadRxData() << 16) |
                              ((uint32_t) UART_ReadRxData() <<  8) |
                              ((uint32_t) UART_ReadRxData() <<  0);
                rs485_tx(MASTER_ADDRESS, UART_WRITE, curr_state);   // send back confirmation
                fan_set_state(ctrl_state, 0);
                //fan_set_state(ctrl_state, TOUT_FAN_SET);
                curr_state = ctrl_state;
                
            }                        
            // Clear RX buffer after processing data
            UART_ClearRxBuffer();
            timer_comm = 0;
        } else if (rx_buff_size != 0) {
            // If buffer isn't full or empty, clear the buffer if a timeout occurs
            if (timer_comm == 0) {
                // Set timer if it hasn't been started yet
                timer_comm = stopwatch_start();
            } else {
                if (stopwatch_elapsed_ms(timer_comm) >= TOUT_RX_COMM) {
                    // Comm timeout: reset timer and clear RX buffer
                    timer_comm = 0;
                    UART_ClearRxBuffer();    
                }
            }
        }
       
        old_state = curr_state;   
#endif // SLAVE

#ifdef IS_MASTER 
        
        CyDelay(5000);
        master_write_all(0);
        CyDelay(5000);
        uint32_t state_test;
        for (uint8_t cell = 0; cell < NUM_CELLS; cell++) {
        //for (uint8_t cell = 7; cell < 8; cell++) {
            state_test = 0;
            for (uint8_t fan = 0; fan < FANS_PER_CELL; fan++) {
                state_test |= (1 << fan);
                master_write_cell(cell, state_test);
                CyDelay(100);
                //uint32_t read_back = master_read_cell(cell);
                //while (read_back != state_test) {
                    //TODO add timeout
                //}   
            }
        }  
#endif
    }
}