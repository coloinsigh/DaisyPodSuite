#!/usr/bin/env python3
import math

"""
Intended to mathematically define the locations of all switches in an isohex Hayden layout.
The output is used to update the individual buttons in the PCB editor inside KiCad

The layout of the grid is to have 2 "satellite" buttons on top and bottom rows, for ancillary
functions. The rest of the buttons are arranged in: 5-6-6-6-5, totalling 32 buttons
This maximizes the available pins on the MCP23017 breakout boards

Some notes:
- Fitting an array of points in the Hayden arrangement within a fixed envelope requires
a rotation, dependent upon the button pitch; 32 degrees was found to achieve this
- A pitch of 16mm was previously found to be
comfortable.
- A centre points of (100,100) was arbitrarily chosen in the KiCad PCB editor.
- Note that the central location of the optimum button is returned. If the buttons
themselves are user-defined, they will often be centered to Pin1 - this may require a 
vertical offset. Any other rotation or offset should be accounted for by adjusting the
CENTER_X and CENTER_Y values
- Additional buttons can be defined at fixed points within grid_def; if doing so, respect 
the half offset between odd and even rows. Below, odd rows are defined as n; even 
rows are defined as n + 0.5, for integers of n

"""

# Design parameters
PITCH = 16.0                              # Back to the generous 16mm spacing!
ROW_HEIGHT = PITCH * (math.sqrt(3) / 2)   # ~13.856mm vertical distance between rows
CENTER_X = 100.0                          # PCB Center X
CENTER_Y = 100.0                           # PCB Center Y
ANGLE_DEG = -32.0                         # Angle of skew pointing extremes into the corners

# Convert angle to radians for the rotation matrix
theta = math.radians(ANGLE_DEG)

# Define the isometric Hayden grid
grid_def = [
    (-3, [0.0, 1.0],                            "Top Satellites"),
    (-2, [-1.5, -0.5, 0.5, 1.5, 2.5],           "Row 1"),
    (-1, [-2.0, -1.0, 0.0, 1.0, 2.0, 3.0],      "Row 2"),
    ( 0, [-2.5, -1.5, -0.5, 0.5, 1.5, 2.5],     "Row 3 (Center)"),
    ( 1, [-3.0, -2.0, -1.0, 0.0, 1.0, 2.0],     "Row 4"),
    ( 2, [-2.5, -1.5, -0.5, 0.5, 1.5],          "Row 5"),
    ( 3, [-1.0, 0.0],                           "Bottom Satellites")
]

print(f"KiCad Absolute Coordinates ({PITCH}mm Pitch, {ANGLE_DEG} Deg Tilt)")
sw_num = 1

for row_idx, x_mults, row_name in grid_def:
    # Calculate unrotated Y
    y = row_idx * ROW_HEIGHT
    
    for x_mult in x_mults:
        # Calculate unrotated X
        x = x_mult * PITCH
        
        # Apply 2D Rotation Matrix
        x_rot = x * math.cos(theta) - y * math.sin(theta)
        y_rot = x * math.sin(theta) + y * math.cos(theta)
        
        # Translate to 100x100 board center
        final_x = CENTER_X + x_rot
        final_y = CENTER_Y + y_rot
        
        print(f"SW{sw_num:02d} [{row_name:17}] -> X: {final_x:7.3f} mm,  Y: {final_y:7.3f} mm")
        sw_num += 1
