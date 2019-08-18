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
#include "project.h"

#define NUM_ROWS 16
#define NUM_COLS 16

uint8_t curr_frame[NUM_ROWS][NUM_COLS];  
uint8_t neighbor_frame[NUM_ROWS][NUM_COLS];  

// Returns shift added to a col index and handles wrap around
uint8_t addx(uint8_t index, uint8_t shift) {
    if (index + shift > NUM_COLS) {
        return index + shift - NUM_COLS;  
    } else if (index + shift < 0) {
        return index + shift + NUM_COLS;   
    }
    else {
        return index + shift;   
    }
}

// Returns shift added to a row index and handles wrap around
uint8_t addy(uint8_t index, uint8_t shift) {
    if (index + shift > NUM_ROWS) {
        return index + shift - NUM_ROWS;  
    } else if (index + shift < 0) {
        return index + shift + NUM_ROWS;   
    }
    else {
        return index + shift;   
    }
}

// Returns number of neighbors for a given (row,col) point of the curr_frame
int get_neighbors(uint8_t row, uint8_t col) {
    uint8_t count;
    for (int x = -1; x < 2; x++) {
        for (int y = -1; y < 2; y++) {
            if (y || x) {
                if (curr_frame[addx(row,x)][addy(col,y)]) count++;
            }
        }
    }
    return count;
}

// Updates the current frame based on Game of Life Rules
void update_frame(uint8_t curr_frame[NUM_ROWS][NUM_COLS]) {
    // Count number of neighbors in current frame
    for (int x = 0; x < NUM_ROWS; x++) {
        for (int y = 0; y < NUM_COLS; y++) {
            neighbor_frame[x][y] = get_neighbors(x, y);
        }
    }
    
    // Apply Game of Life Rules to next frame
    for (int x = 0; x < NUM_ROWS; x++) {
        for (int y = 0; y < NUM_COLS; y++) {
            if (curr_frame[x][y]) {
                // Handle Living Cell Rules
                if (neighbor_frame[x][y] < 2 ||
                    neighbor_frame[x][y] > 3) {
                    neighbor_frame[x][y] = 0;  // Die if less than 2 or more than 3 living neighbors
                } else {
                    curr_frame[x][y] = 1;  // Stay alive if 2 or 3 living neighbors
                }
            } else {
                // Handle Dead Cell Rule
                if (neighbor_frame[x][y] == 3) {
                    neighbor_frame[x][y] = 1;  // Come to life if 3 neighbors
                } else {
                    curr_frame[x][y] = 0;  // Stay dead otherwise
                }
            }
        }
    }
}

/* [] END OF FILE */
