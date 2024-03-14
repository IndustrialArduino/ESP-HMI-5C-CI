Display Touch Calibration

The display used in ESP HMI is a 7” TFT with resistive touch. 
After the display is set up, the touch feature may need to be calibrated for better accuracy. 
Calibrating the touch feature of a display can be done in just a few simple steps.

This Arduino code is an example of how to use the XPT2046 touchscreen controller with an Arduino board. 
This code reads touch input from the XPT2046 touchscreen controller and outputs the touch coordinates and pressure level to the serial monitor for debugging purposes. The XPT2046 is commonly used in resistive touchscreen displays.

Here's a breakdown of what each part of the code does,

Include Libraries: The code includes the necessary libraries for the XPT2046 touchscreen (XPT2046_Touchscreen) and the SPI communication (SPI).

Define Pins: It defines the pins used for SPI communication (MOSI_PIN, MISO_PIN, SCK_PIN) and the chip select pin (CS_PIN) for the touchscreen. Additionally, it defines the touch interrupt pin (TIRQ_PIN), which is used for interrupt-based touch detection.

Touchscreen Object Initialization: initializes the touchscreen object (ts) with the defined chip select the pin and touch the interrupt pin. Optionally, it can also specify the SPI interface (SPI1) if an alternate SPI port is used. The rotation of the touchscreen is set to 2 (landscape mode).

Setup Function:
It initializes serial communication for debugging purposes.
It initializes the SPI communication.
It initializes the touchscreen.
It sets the rotation of the touchscreen.

Loop Function:
It continuously checks if there's a touch detected using the tirqTouched() function, which relies on the touch interrupt pin for faster detection.
If a touch is detected (if (ts.touched())), it retrieves the touch coordinates using the getPoint() function.
It prints the pressure, x-coordinate, and y-coordinate of the touch point to the serial monitor.
It introduces a delay of 100 milliseconds before the next iteration.

Execute the calibration code now. The corresponding coordinates are displayed on the serial monitor when you touch the display. Next, make an accurate attempt to touch each of the four corners; the serial monitor will display the pressure and corresponding coordinates. The display can be calibrated based on this.