arduino-PlantWaterer
====================
by: Nathan Gardner <nathan@factry8.com>
This is my first real Arduino project!

Uses a soil moisture sensor, and servo to water plants when the soil is dry.

The soil moisture sensor is simply two nails stuck in the ground, and we
measure resistance in the soil. The more moisture, the less resistance.
The servo is connected to a homemade pinch valve, using lytex tubing.
I am using a rain barrel as my water source, and rely on gravity to move water.


How the code works:

First it runs a startup process, outputting a message to Serial and closing
the water valve. Then it starts the soil moisture process.

The soil moisture process checks the soil mositure on a set interval.
If the soil is wet, it waits, then checks the soil again.
If the soil is dry, it switches to the watering process.

The watering processes rotates a servo to "open", waits for a set time, then
rotates back to the "closed" position. It pauses while the water soaks in, then
switches back to the soil moisture processes.

This causes it to water for a set amount of time, then check the moisture. If it
needs more, it waters again. Otherwise it just waits for the soil to dry.

All timers can easily be changed using the variables in the header.
