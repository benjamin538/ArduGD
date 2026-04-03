#pragma once
#include <Windows.h>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

/*
static class Arduino to communicate with port
*/
class Arduino {
private:
    inline static HANDLE hComm = INVALID_HANDLE_VALUE;
public:
    /*opens port on `port` saved value if saved value not exists / == 0
    returns TRUE if successfully opened and FALSE if something gone wrong*/
    static BOOL open(int baudRate) {
        int portNum = Mod::get()->getSavedValue<int>("port");
        if (!portNum || portNum == 0) {
            log::error("cant find port");
            return FALSE;
        }
        const char* portFile = std::format("\\\\.\\COM{}", portNum).c_str(); 
        hComm = CreateFileA(portFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hComm == INVALID_HANDLE_VALUE) {
            log::error("port not opened");
            return FALSE;
        }
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 20;
        timeouts.ReadTotalTimeoutMultiplier = 1;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 1;
        SetCommTimeouts(hComm, &timeouts);
        DCB dcbStruct = {0};
        dcbStruct.BaudRate = baudRate;
        dcbStruct.StopBits = ONESTOPBIT;
        dcbStruct.DCBlength = sizeof(dcbStruct);
        dcbStruct.fDtrControl = DTR_CONTROL_DISABLE;
        BOOL status = GetCommState(hComm, &dcbStruct);
        if (status == FALSE) {
            log::error("getcommstate failed, closing");
            CloseHandle(hComm);
            return FALSE;
        }
        SetCommState(hComm, &dcbStruct);
        log::info("successfuly opened");
        PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);
        return TRUE;
    }

    /*overloaded function where YOU specifying the port*/
    static BOOL open(int port, int baudRate) {
        const char* portFile = std::format("\\\\.\\COM{}", port).c_str(); 
        hComm = CreateFileA(portFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hComm == INVALID_HANDLE_VALUE) {
            log::error("port not opened");
            return FALSE;
        }
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = 0;
        SetCommTimeouts(hComm, &timeouts);
        DCB dcbStruct = {0};
        dcbStruct.BaudRate = baudRate;
        dcbStruct.ByteSize = 8;
        dcbStruct.Parity = NOPARITY;
        dcbStruct.StopBits = ONESTOPBIT;
        SetCommState(hComm, &dcbStruct);
        BOOL status = GetCommState(hComm, &dcbStruct);
        if (status == FALSE) {
            log::error("getcommstate failed, closing");
            CloseHandle(hComm);
            return FALSE;
        }
        log::info("successfuly opened");
        return TRUE;
    }

    /*closes port if it was opened, if not, just prints the error*/
    static BOOL close() {
        if (hComm != INVALID_HANDLE_VALUE)  {
            CloseHandle(hComm);
            log::info("closed");
            hComm = INVALID_HANDLE_VALUE;
            return TRUE;
        } else {
            log::error("already closed");
            return FALSE;
        }
    }

    /*prints `str` in port, returns TRUE if successfully printed and FALSE if something gone wrong*/
    static BOOL write(const char* str) {
        if (hComm != INVALID_HANDLE_VALUE) {
            DWORD bytesWritten = 0;
            BOOL status = WriteFile(hComm, str, strlen(str), &bytesWritten, NULL);
            if (status) log::info("written to port"); 
            else log::error("cant write to port for unknown reason");
            return status;
        } else {
            log::error("cant write to non opened port");
            return FALSE;
        }
    }

    /*returns TRUE if opened, FALSE if not*/
    static BOOL isOpened() {
        return hComm != INVALID_HANDLE_VALUE;
    }
    
    /*reads port and returns bytes
    max size of buffer is 128 bytes, you cant complain about it*/
    static std::string read() {
        if (hComm != INVALID_HANDLE_VALUE) {
            DWORD eventMask;
            DWORD bytesRead;
            char buff[128] = {0};
            BOOL status = SetCommMask(hComm, EV_RXCHAR);
            if (!status) {
                log::error("failed to set mask");
                return "";
            }
            status = WaitCommEvent(hComm, &eventMask, NULL);
            if (status) {
                if (eventMask && EV_RXCHAR) {
                    ReadFile(hComm, buff, sizeof(buff) - 1, &bytesRead, NULL);
                    if (bytesRead > 0) {
                        return std::string(buff, bytesRead);
                    } else return "";
                }
            } else {
                log::error("wait failure");
                return "";
            }
        } else {
            log::error("cant read non opened port");
            return "";
        }
        return "";
    }
};