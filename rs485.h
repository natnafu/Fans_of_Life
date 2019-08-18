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

#define UART_READ  0
#define UART_WRITE 1
    
void rs485_tx(uint8_t addr, uint8_t read_write, uint32_t state) {
    UART_PutChar(addr);
    UART_PutChar(read_write);
    UART_PutChar((uint8_t) (state >> 24));
    UART_PutChar((uint8_t) (state >> 16));
    UART_PutChar((uint8_t) (state >>  8));
    UART_PutChar((uint8_t) (state >>  0));  
}

/* [] END OF FILE */
