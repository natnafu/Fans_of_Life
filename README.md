The Fans of Life: interactive grid of 256 fans that play Conway's Game of Life.

1x Master and 8x Cells:
- each cell controls and reads the state of 32 fans.
- each cell compares commanded state to tach signal to tell if a human has changed the state.
- if a human has changed the state, the cell will correct the commanded state (human spins, cell continues spinning).
- the master gets the state of all fans from cells, calculates next generation, then sends out commands to all cells.
