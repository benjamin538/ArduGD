# ArduGD

![Mod Version](https://api.geode-sdk.org/v1/mods/benjamin538.ardugd/status_badge?stat=version)
![Downloads](https://api.geode-sdk.org/v1/mods/benjamin538.ardugd/status_badge?stat=downloads)
![GD Version](https://api geode-sdk.org/v1/mods/benjamin538.ardugd/status_badge?stat=gd_version)
![Geode Version](https://api.geode-sdk.org/v1/mods/benjamin538.ardugd/status_badge?stat=geode_version)

Mod that provides support for Arduino / ESP right in game!

**Windows only!**

## What it adds:

- Better API than Win32 calls
- Built-in serial port monitor where you can send data, clear monitor and copy data
- Settings for port number and baud rate

# Example:

```cpp
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <arduino.hpp>

// Port must be opened from menu, but nobody stops you from calling Arduino::open(portNum, baudRate);

// Modifying Menulayer
class $modify(MenuLayer) {
    bool init() {
        // Initializing MenuLayer
        if (!MenuLayer::init()) return false;
        // Writing to opened port
        if (Arduino::isOpened()) Arduino::write("Hello from GD!");
        // Returning successful initialization
        return true;
    }
}
```