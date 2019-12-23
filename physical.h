/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

// For master (full grid)
#define NUM_ROWS        16
#define NUM_COLS        16
#define NUM_CELLS       8   // number of slave cells

// For cells
#define CELL_ROWS       4   // number of rows in a cell
#define CELL_COLS       8   // number of columns in a cell
#define FANS_PER_CELL   (CELL_ROWS * CELL_COLS)  // number of fans in each cell

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

/* [] END OF FILE */
