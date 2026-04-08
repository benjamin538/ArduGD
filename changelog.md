# 1.1.0

- `Arduino::read()` now just reads file and doesnt wait `WaitCommEvent()`
- Removed filter from input in serial monitor 
- Fixed some undefined behaviour with threads
- Fixed `onQuit()` hook, now its just calls `MenuLayer::onQuit` when port is closed

# 1.0.0

Initial release