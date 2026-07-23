# Engineering Lessons Learned & Debugging Log

## 1. Open-Drain I2C Bus Locking
* **Problem:** In certain instances, especially as the number of peripherals on the I2C bus increases in number calls to `ReadDataAtAdress()` hung indefinitely, completely halting execution.
* **Diagnosis:** The I2C peripheral operates in open-drain. When a line is pulled to 0V, the I2C peripheral can't actively push the line back to 3.3V. Parasitic capacitance in some cases can be overcome, when dealing with a small number of devices. I have observed 8 momentary push buttons can operate well.
* **Solution:** Adding physical pull-up resistors between SDA and VCC and SCL and VCC can return the I2C lines to 3.3V. In tests, 4.7kΩ resistors are sufficient. **Future prototypes will include physical pull-up resistors.**

## 2. Button bouncing
* **Problem:** Single, sustained physical button presses were being registered as multiple "ghost" notes, especially when being held under other notes. This is not the desired outcome where a sustained note should sound like a single note regardless of other note activity.
* **Diagnosis:** Button multiplexing is handled by the MCP23017 within the `main()` loop. Due to the high refresh rate, some mechanical key chatter was detected as legitimate button presses, triggering the envelope repeatedly.
* **Solution:** Replacing blocking delays with non-blocking delays, which are a function of time since last button press. Only when the time between button presses is above some global threshold is a button state change treated as a legitimate change. The time delay is set at 5ms by default, which is sufficiently high to prevent chatter, while low enough not to impact the latency of a key press discernably.  
