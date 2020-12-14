#include "project.h"

#define GPIOXA      0
#define GPIOXB      1
#define SPI_ENABLE  0
#define SPI_DISABLE 1

// Registers for MCP23S17
#define ADDR_MCP23S17 0b01000000 // address (pull all Ax pins low)
#define ADDR_IOCON    0x05       // configuration register (keep default)
#define ADDR_IODIRA   0x00       // GPIO port A direction register 
#define ADDR_IODIRB   0x01       // GPIO port B direction register 
#define ADDR_PORTA    0x12       // GPIO port A register
#define ADDR_PORTB    0x13       // GPIO port A register

/*
Writes two bytes via the SPIM device.
Byte dataA is written to register regA
The MCP23S17 will increment its addess pointer to the next slot (regB)
Byte dataB will be written to this register (regB)
*/
void gpiox_send(uint8_t gpiox, uint8_t regA, uint8_t dataA, uint8_t dataB) {
    // Enable SPI peripheral
    if (gpiox == GPIOXA) nCS_GPIOXA_Write(SPI_ENABLE);
    if (gpiox == GPIOXB) nCS_GPIOXB_Write(SPI_ENABLE);
    
    // Send data
    SPIM_GPIOX_WriteByte(ADDR_MCP23S17); // IC hardware address   
    SPIM_GPIOX_WriteByte(regA);          // register A address 
    SPIM_GPIOX_WriteByte(dataA);         // register A data
    SPIM_GPIOX_WriteByte(dataB);         // register B data
    while (0u == (SPIM_GPIOX_ReadTxStatus() & SPIM_GPIOX_STS_SPI_DONE)); // wait for TX to finish
    
    // Disable SPI peripheral
    if (gpiox == GPIOXA) nCS_GPIOXA_Write(SPI_DISABLE);
    if (gpiox == GPIOXB) nCS_GPIOXB_Write(SPI_DISABLE);
}

void gpiox_init(void) {
    SPIM_GPIOX_Start();
    
    // Set all as outputs (IODIR = 0), default are inputs
    gpiox_send(GPIOXA, ADDR_IODIRA, 0, 0);
    gpiox_send(GPIOXB, ADDR_IODIRA, 0, 0);
}
