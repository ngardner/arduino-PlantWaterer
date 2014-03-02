arduino-PlantWaterer
====================

Uses a soil moisture sensor, and servo to water plants when the soil is dry.

This is my first real Arduino project. It checks the soil mositure on a set interval.
If the soil is wet, it switches to a watering process. Otherwise it starts over.

The watering processes rotates a servo to "open" for a set interval, then rotates
back to "closed". It then pauses while the water soaks in. It then switches back
to the soil moisture processes.

This causes it to water for a set amount of time, then check the moisture. If it
needs more, it waters again. This continues until the soil is wet, in which then
it continues to check to mositure until its dry.

All timers can easily be changed using the variables in the header.
