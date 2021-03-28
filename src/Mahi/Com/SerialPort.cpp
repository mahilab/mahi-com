// #include <Mahi/Com/SerialPort.hpp>
#include <Mahi/Com.hpp>
#include "Detail/rs232.hpp"

using namespace mahi::util;

namespace mahi {
namespace com {

SerialPort::SerialPort() :  
    is_open_(false) 
{
}

SerialPort::~SerialPort() {
    if (is_open())
        close();
}

bool SerialPort::is_open() const {
    return is_open_;
}

// Platform specific functions

// WINDOWS or LINUX
#if defined(WIN32) || defined(__linux__)
bool SerialPort::open(Port port, std::size_t baudrate, std::string mode) {
    port_= port;
    baudrate_ = baudrate;
    mode_ = mode;

    if (is_open())
        close();
    if (RS232_OpenComport(static_cast<int>(port_), static_cast<int>(baudrate_), mode_.c_str()) == 1)
        return false;
    else {
        is_open_ = true;
        return true;
    }
}

bool SerialPort::send_data(unsigned char* data, std::size_t size) {
    if (RS232_SendBuf(static_cast<int>(port_), data, static_cast<int>(size)) == 1)
        return false;
    else
        return true;
}

int SerialPort::receive_data(unsigned char* data, std::size_t size) {
    return RS232_PollComport(static_cast<int>(port_), data, static_cast<int>(size));
}

void SerialPort::flush_RX() {
    RS232_flushRX(static_cast<int>(port_));
}

void SerialPort::flush_TX() {
    RS232_flushTX(static_cast<int>(port_));
}

void SerialPort::flush_RXTX() {
    RS232_flushRXTX(static_cast<int>(port_));
}

bool SerialPort::close() {
    RS232_CloseComport(static_cast<int>(port_));
    return true;
}

// MACOS
#else
bool SerialPort::open(std::string port, std::size_t baudrate, std::string mode) {
    port_= port;
    baudrate_ = baudrate;
    mode_ = mode;

    if (is_open())
        close();
    if (RS232_OpenComport(port_, static_cast<int>(baudrate_), mode_.c_str()) == 1)
        return false;
    else {
        is_open_ = true;
        return true;
    }
}

bool SerialPort::send_data(unsigned char* data, std::size_t size) {
    if (RS232_SendBuf(port_, data, static_cast<int>(size)) == 1)
        return false;
    else
        return true;
}

int SerialPort::receive_data(unsigned char* data, std::size_t size) {
    return RS232_PollComport(port_, data, static_cast<int>(size));
}

void SerialPort::flush_RX() {
    RS232_flushRX(port_);
}

void SerialPort::flush_TX() {
    RS232_flushTX(port_);
}

void SerialPort::flush_RXTX() {
    RS232_flushRXTX(port_);
}

bool SerialPort::close() {
    RS232_CloseComport(port_);
    return true;
}

#endif

} // namespace mahi
} // namespace com
