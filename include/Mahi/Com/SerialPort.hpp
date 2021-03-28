// MIT License
//
// MEL - Mechatronics Engine & Library
// Copyright (c) 2018 Mechatronics and Haptic Interfaces Lab - Rice University
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// Author(s): Evan Pezent (epezent@rice.edu)

#pragma once
// #include <Mahi/Core/NonCopyable.hpp>
// #include <Mahi/Communications/SerialSettings.hpp>
#include <Mahi/Util.hpp>
#include <Mahi/Com.hpp>
#include <string>

namespace mahi {
namespace com {

#if defined(__linux__) || defined(__FreeBSD__)
enum Port {
    ttyS0 = 0,
    ttyS1, 
    ttyS2,
    ttyS3,
    ttyS4, 
    ttyS5, 
    ttyS6,
    ttyS7,
    ttyS8,
    ttyS9,
    ttyS10,
    ttyS11,
    ttyS12,
    ttyS13,
    ttyS14,
    ttyS15,
    ttyUSB0,
    ttyUSB1,
    ttyUSB2,
    ttyUSB3,
    ttyUSB4,
    ttyUSB5,
    ttyAMA0,
    ttyAMA1,
    ttyACM0,
    ttyACM1,
    rfcomm0,
    rfcomm1,
    ircomm0,
    ircomm1,
    cuau0,
    cuau1,
    cuau2,
    cuau3,
    cuaU0,
    cuaU1,
    cuaU2,
    cuaU3,
};
#elif defined(WIN32)
enum Port {
    COM1  = 0,
    COM2  = 1,
    COM3  = 2,
    COM4  = 3,
    COM5  = 4,
    COM6  = 5,
    COM7  = 6,
    COM8  = 7,
    COM9  = 8,
    COM10 = 9,
    COM11 = 10,
    COM12 = 11,
    COM13 = 12,
    COM14 = 13,
    COM15 = 14,
    COM16 = 15
};
#else
typedef std::string Port;
#endif

/// Interface to RS-232 Serial Port
class SerialPort : util::NonCopyable {
public:

    /// Default constructor. Creates an empty (invalid) serialport
    SerialPort();

    /// Destructor
    ~SerialPort();

    /// Returns true if SerialPort is open
    bool is_open() const;

    /// Opens communication on specified port
    bool open(Port port, std::size_t baudrate = 9600, std::string mode = "8N1");

    /// Closes communication on specified port
    bool close();

    /// Sends data
    bool send_data(unsigned char* data, std::size_t size);

    /// Receives data
    int receive_data(unsigned char* data, std::size_t size);

    // Flush data received but not read
    void flush_RX();

    /// Flush data written but not transmitted
    void flush_TX();

    /// Flushes both data received but not read, and data written but not transmitted.
    void flush_RXTX();


private:

    bool is_open_;
    Port port_;
    std::size_t baudrate_;
    std::string mode_;
};

} // namespace mahi
} // namespace com
