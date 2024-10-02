# WiFi6_PwrTest_Peer

## Tested with ESP-IDF v5.2.1

### Hardware to use
1. ESP32-C6-DevKitC-1

#### Function
1. Connect a serial terminal software via the UART output to read the log (orintf) messages from the device
2. At start-up the device will print its MAC address via the serial port. Use this MAC address in the other device (SDK Config)
3. Once a ESP-NOW data package has been received from the WiFi6_PwrTest peer the RGB led will shortly blink