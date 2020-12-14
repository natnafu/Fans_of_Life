#pragma once
#include "project.h"

#define UART_READ       0   // fan read command
#define UART_WRITE      1   // fan write command
#define UART_CONFIG     2   // cell configure command

#define CONTROLLER_ADDRESS  8
#define PACKET_SIZE     6

/* 
    All communication will be 6 byes sent in this order:
    1. Receiver's address
    2. read or write
    3. fans 24-31
    4. fans 16-23
    5. fans 8-15
    6. fans 0-7
    
    Receiver processes data once it detects a full buffer of *5 bytes.
    *NOTE: UART hardware detect-to-buffer only passed data bytes to buffer, not address.
*/  
void rs485_tx(uint8_t addr, uint8_t read_write, uint32_t state) {
    UART_ClearRxBuffer();   // make sure there is space for the response
    uint8_t tx_data[PACKET_SIZE] = {
        addr, 
        read_write,
        ((uint8_t) (state >> 24)),
        ((uint8_t) (state >> 16)),
        ((uint8_t) (state >>  8)),
        ((uint8_t) (state >>  0))
    };
    UART_SetTxAddressMode(UART_SET_MARK);
    UART_PutArray(tx_data, PACKET_SIZE);
    UART_SetTxAddressMode(UART_SET_SPACE);
}
