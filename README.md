# Swapper3D_ServoSetup
Swapper3D Setup Firmware

Author: BigBrain3D

License: AGPLv3

This Arduino sketch is used to control a set of servos for the Swapper3D printer setup. The firmware enables the manual control and adjustment of various parts of the 3D printer, each of which is controlled by a dedicated servo.
Major Components:

    LCD Display: This provides a visual interface to see the current status and make adjustments.
    8 Servos: These control different aspects of the printer's operation. These include the rotation and height of the tool, tool lock, QuickSwap Hotend Lock, holder rotation, cutter rotation, cutter action, and waste cup action.
    PWM Servo Driver: This is used to control the servos.
    EEPROM: This is used to store the current configuration and settings of the servos, so they can be reloaded on restart.
    Button Interface: This is used to interact with the device and adjust settings, the readings from the buttons are used to manipulate the servo configurations.

Functionalities:

    The code includes setup and loop routines.
    In the setup routine, it initializes the LCD and the PWM Servo Driver, sets the servos to their starting positions, and loads saved settings from the EEPROM.
    In the loop routine, it reads button presses to adjust the angle of the current servo, switch between servos, or save the current settings to EEPROM. It also updates the LCD display and servo positions based on these interactions.

EEPROM Configuration:

Servo adjustment settings are stored in EEPROM. The values are mapped from a range of -20 to 20 (adjustment angle) to 100 to 140 (stored value). If the settings haven't been saved before, they are set to 0 (mapped to 120).
Note:

It's important to understand that incorrect setup or misuse can lead to mechanical problems. For instance, leaving the setup firmware on the controller and then powering on without proper assembly can cause the arms and parts to crash and break.