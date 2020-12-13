# Fans of Life
Interactive grid of 256 fans that play Conway's Game of Life.
>Made for and tested on PSoC5 (CY8CKIT-059).

## Description
This art project plays out [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)
on a grid of 256 fans. A spinning fan is "alive" and a stationary fan is "dead".

If someone manually spins a fan, it will continue spinning (living).  
If someone manually stops a fan, it will stay stopped (dead).

[See a video of the project here.](https://vimeo.com/429323547)

<img src="fol_1.JPG" height="400" />

## Hardware/Firmware
The system is setup with 1 controller and 8 cells.

Cells:
* Each cell controls and reads the state of 32 fans.
* Each cell compares the commanded state to the read state to tell if a fan was manually spun.
* If a fan was manually spun, the cell will correct the commanded state (i.e. human spins fan, cell continues spinning it).

Controller:
* The controller gets the state of all the fans from the cells and constructs the full grid.
* Using the full grid, the controller calculates the next generation and sends out commands to all cells.