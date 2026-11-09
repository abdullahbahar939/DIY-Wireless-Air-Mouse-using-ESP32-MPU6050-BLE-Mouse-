# DIY-Wireless-Air-Mouse-using-ESP32-MPU6050-BLE-Mouse-

I’ll show you how to build a custom wireless 3D motion-sensing Air Mouse using an ESP32 microcontroller and an MPU-6050 6-axis gyroscope/accelerometer module. This project acts as a native Bluetooth Low Energy (BLE) Human Interface Device (HID), allowing you to control your computer, laptop, or Android device cursor seamlessly in mid-air—no extra receivers needed!
📌 Project Overview & Highlights
Native Bluetooth HID: Connects directly via BLE as a standard wireless mouse.
6-Axis Motion Control: MPU6050 sensor reads pitch and roll to translate hand gestures into smooth cursor movements.
Physical Click Buttons: Dedicated tactile push buttons for Left Click and Right Click.
Low Power & Compact: Optimized pin layout suitable for wearable finger/glove mounting or handheld enclosures.
🛠️ Components & Hardware Required
ESP32 Development Board (ESP32-WROOM-32 / DevKit v1)
MPU-6050 6-Axis Gyroscope / Accelerometer Sensor
2x Tactile Push Buttons (Left & Right Mouse Clicks)
Breadboard & Jumper Wires (or Custom PCB / Perfboard)
Micro-USB / USB-C Cable (for power & programming)
🔌 Wiring & Pin Connections
Module / Component
ESP32 GPIO Pin
Description
MPU6050 VCC
3.3V
 Power Supply
MPU6050 GND
GND
 Ground
MPU6050 SCL
GPIO 22
 I2C Clock Line
MPU6050 SDA
GPIO 21
 I2C Data Line
Left Click Button
GPIO 23
 Pull-Up Input (Connects to GND when pressed)
Right Click Button
GPIO 19
 Pull-Up Input (Connects to GND when pressed)

📚 Required Libraries & GitHub Links
ESP32 BLE Mouse Library (by T-vK)
Allows the ESP32 to emulate a Bluetooth HID Mouse.
GitHub Repository: https://github.com/T-vK/ESP32-BLE-Mouse
Adafruit MPU6050 Library
Handles I2C communication and motion data extraction.
GitHub Repository: https://github.com/adafruit/Adafruit_MPU6050
Adafruit Unified Sensor Library (Prerequisite for MPU6050)
GitHub Repository: https://github.com/adafruit/Adafruit_Sensor
🚀 Step-by-Step Making Process
Step 1: Software & Environment Setup
Open Arduino IDE.
Install the ESP32 Board Package via Tools > Board > Boards Manager (search for esp32 by Espressif Systems).
Download the ESP32 BLE Mouse repository as a ZIP file from GitHub and install it via Sketch > Include Library > Add .ZIP Library...
Go to Tools > Manage Libraries..., search for Adafruit MPU6050, and click Install All (this automatically installs dependent sensor libraries).
Step 2: Circuit Hardware Assembly
Connect the MPU6050 to the ESP32 using the I2C bus: SDA to GPIO 21 and SCL to GPIO 22.
Connect VCC to 3.3V and GND to GND.
Wire the Left Click Button between GPIO 12 and GND.
Wire the Right Click Button between GPIO 14 and GND.
Step 3: Code Upload & Calibration
Connect your ESP32 to your PC via USB.
Select your specific ESP32 board and COM port under Tools.
Upload the Air Mouse firmware sketch.
Keep the assembly stationary on a flat surface during initial startup for accurate gyro zero-calibration.
Step 4: Pairing & Testing
Enable Bluetooth on your PC, tablet, or smartphone.
Search for new Bluetooth devices and select ESP32 Air Mouse.
Once paired, tilt the assembly to move your mouse cursor and press the buttons to perform left and right clicks.
