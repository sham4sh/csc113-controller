# csc113-controller

Final project for CSC113; this repository contains the codebase for a wireless control system for an electronic bike using two ESP32 microcontrollers with Arduino framework. This is the repository for the standalone controller, the onboard processor's code can be found at 

https://github.com/mike-ilya/DC_Motor_Test

The system consists of an onboard computer with several attached sensors, communicating with a remote controller via an MQTT server. A joystick is used for control of the motors, and the controller may be put into sleep for manual control via pushbutton.

To view user dashboard, import flows.json to a node-red flow and open localhost:1880/ui. Note this display requires the mode-red-dashboard module.

Developed by Joshua Shamash and Michael Ilyajanov.
