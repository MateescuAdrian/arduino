# ESP32 Robot Remote Control Project

This project contains various Arduino sketches and HTML control panels for an ESP32-based robot. The robot utilizes an ESP32 microcontroller, an H-Bridge to control two DC motors, and a servo motor for steering. It also includes an HTTP server on the ESP32 to receive commands and an HTML frontend for remote control via keyboard or gamepad.

## Project Structure

*   **`html/`**: Contains the frontend HTML interfaces for controlling the robot.
    *   `echtpower.html`: A web interface allowing control via ZQSD keys or a connected gamepad. Uses `m1` and `m2` parameters.
    *   `echtpower2.html`: An updated web interface with diagonal movement support via keyboard combos, and a speed slider. Sends `m1` (motor speed) and `servo` (steering angle) parameters.
*   **ESP32 Sketches**:
    *   **`webridge/webridge.ino`**: A sketch for controlling two DC motors and a steering servo. It sets up an `ESPAsyncWebServer` and handles the `/drive` route with `m1` (motor speed) and `servo` (steering angle) parameters. Connects to WiFi and outputs status to WebSerial.
    *   **`mrauW/mrauW.ino`**: Another combined sketch similar to `webridge.ino`. It controls two motors using a single `m1` parameter (for forward/backward speed on both motors) and a `servo` parameter for steering. It uses direct PWM (`ledcWrite`) for the servo instead of the `ESP32Servo` library.
    *   **`m1m2working/m1m2working.ino`**: A sketch focused on controlling two DC motors using `m1` and `servo` parameters (though servo logic is commented out).
    *   **`mrau/mrau.ino`**: A sketch for independent control of two DC motors using `m1` and `m2` parameters.
    *   **`2motors/2motors.ino`**: Similar to `mrau.ino`, controls two DC motors independently with `m1` and `m2` parameters.
    *   **`wifi/wifi.ino`**: A basic sketch demonstrating WiFi connection and setting up a simple web server with WebSerial, listening on a `/motor1` route.
    *   **`servo/servo.ino`**: A simple test sketch for moving a servo motor using the `ESP32Servo` library.
    *   **`Sweep/Sweep.ino`**: The standard Arduino Servo Sweep example sketch.

## Hardware Setup

*   **Microcontroller**: ESP32
*   **Motor Driver**: H-Bridge (e.g., L298N)
    *   Motor 1 (Left) typically connected to IN1 and IN2.
    *   Motor 2 (Right) typically connected to IN3 and IN4.
*   **Steering**: Servo Motor

### Pin Assignments (Typical based on `webridge.ino` and `mrauW.ino`)

*   **H-Bridge IN1**: GPIO 36
*   **H-Bridge IN2**: GPIO 37
*   **H-Bridge IN3**: GPIO 38
*   **H-Bridge IN4**: GPIO 39
*   **Servo Motor**: GPIO 40 (in `webridge.ino`) or GPIO 17 (in `mrauW.ino`)

*Note: Pin assignments may vary slightly between different sketches in the repository. Please refer to the specific `.ino` file you are using.*

## Usage

1.  **Configure WiFi**: Open the desired `.ino` sketch (e.g., `webridge/webridge.ino` or `mrauW/mrauW.ino`) and update the `ssid` and `password` variables with your local WiFi network credentials.
2.  **Upload Sketch**: Upload the sketch to your ESP32 board using the Arduino IDE.
3.  **Find IP Address**: Open the Serial Monitor at `115200` baud rate to find the IP address assigned to the ESP32.
4.  **Open Control Panel**: Open `html/echtpower2.html` (recommended) in a web browser.
5.  **Set IP**: Enter the ESP32's IP address into the input field on the web page.
6.  **Control Robot**: Use the on-screen buttons, your keyboard (Z, Q, S, D), or a connected gamepad to drive the robot.

## Dependencies

The ESP32 sketches rely on the following libraries:

*   `WiFi.h` (Built-in)
*   `Arduino.h` (Built-in)
*   [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
*   [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
*   [WebSerial](https://github.com/ayushsharma82/WebSerial)
*   [ESP32Servo](https://github.com/madhephaestus/ESP32Servo) (used in `webridge.ino` and `servo.ino`)
