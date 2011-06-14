4x4x4 LED Cube Controller
===========================

A display controller for a 4x4x4 LED cube. Based on a atmega8 microcontroller that is connected to 2 8-bit shift registers (74HC595). 

The 2 8-bit shift registers can control one level in the cube at a time (4x4 = 16bits). 
The microcontroller scans through each level in the cube, turning each one 1/4 of the time.
