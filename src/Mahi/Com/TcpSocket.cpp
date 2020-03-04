// #include <Mahi/Com/TcpSocket.hpp>
// #include <Mahi/Com/IpAddress.hpp>
// #include <Mahi/Com/Packet.hpp>
// #include <Mahi/Logging/Log.hpp>
#include <Mahi/Util.hpp>
#include <Mahi/Com.hpp>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#include <basetsd.h>
#ifdef _WIN32_WINDOWS
    #undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif

#ifdef _MSC_VER
    #pragma warning(disable: 4127) // "conditional expression is constant" generated by the FD_SET macro
#endif

namespace
{
    // Define the low-level send/receive flags, which depend on the OS
    #ifdef __linux__
        const int flags = MSG_NOSIGNAL;
    #else
        const int flags = 0;
    #endif
}

using namespace mahi::util;

namespace mahi {
namespace com {
TcpSocket::TcpSocket() :
Socket(Tcp)
{

}


unsigned short TcpSocket::get_local_port() const
{
    if (get_handle() != Socket::invalid_socket())
    {
        // Retrieve informations about the local end of the socket
        sockaddr_in address;
        Socket::AddrLength size = sizeof(address);
        if (getsockname(get_handle(), reinterpret_cast<sockaddr*>(&address), &size) != -1)
        {
            return ntohs(address.sin_port);
        }
    }

    // We failed to retrieve the port
    return 0;
}


IpAddress TcpSocket::get_remote_address() const
{
    if (get_handle() != Socket::invalid_socket())
    {
        // Retrieve informations about the remote end of the socket
        sockaddr_in address;
        Socket::AddrLength size = sizeof(address);
        if (getpeername(get_handle(), reinterpret_cast<sockaddr*>(&address), &size) != -1)
        {
            return IpAddress(ntohl(address.sin_addr.s_addr));
        }
    }

    // We failed to retrieve the address
    return IpAddress::None;
}


unsigned short TcpSocket::get_remote_port() const
{
    if (get_handle() != Socket::invalid_socket())
    {
        // Retrieve informations about the remote end of the socket
        sockaddr_in address;
        Socket::AddrLength size = sizeof(address);
        if (getpeername(get_handle(), reinterpret_cast<sockaddr*>(&address), &size) != -1)
        {
            return ntohs(address.sin_port);
        }
    }

    // We failed to retrieve the port
    return 0;
}


Socket::Status TcpSocket::connect(const IpAddress& remoteAddress, unsigned short remotePort, Time timeout)
{
    // Disconnect the socket if it is already connected
    disconnect();

    // Create the internal socket if it doesn't exist
    create();

    // Create the remote address
    sockaddr_in address = Socket::create_address(remoteAddress.to_integer(), remotePort);

    if (timeout <= Time::Zero)
    {
        // ----- We're not using a timeout: just try to connect -----

        // Connect the socket
        if (::connect(get_handle(), reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1)
            return Socket::get_error_status();

        // Connection succeeded
        return Done;
    }
    else
    {
        // ----- We're using a timeout: we'll need a few tricks to make it work -----

        // Save the previous blocking state
        bool blocking = is_blocking();

        // Switch to non-blocking to enable our connection timeout
        if (blocking)
            set_blocking(false);

        // Try to connect to the remote address
        if (::connect(get_handle(), reinterpret_cast<sockaddr*>(&address), sizeof(address)) >= 0)
        {
            // We got instantly connected! (it may no happen a lot...)
            set_blocking(blocking);
            return Done;
        }

        // Get the error status
        Status status = Socket::get_error_status();

        // If we were in non-blocking mode, return immediately
        if (!blocking)
            return status;

        // Otherwise, wait until something happens to our socket (success, timeout or error)
        if (status == Socket::NotReady)
        {
            // Setup the selector
            fd_set selector;
            FD_ZERO(&selector);
            FD_SET(get_handle(), &selector);

            // Setup the timeout
            timeval time;
            time.tv_sec  = static_cast<long>(timeout.as_microseconds() / 1000000);
            time.tv_usec = static_cast<long>(timeout.as_microseconds() % 1000000);

            // Wait for something to write on our socket (which means that the connection request has returned)
            if (select(static_cast<int>(get_handle() + 1), NULL, &selector, NULL, &time) > 0)
            {
                // At this point the connection may have been either accepted or refused.
                // To know whether it's a success or a failure, we must check the address of the connected peer
                if (get_remote_address() != IpAddress::None)
                {
                    // Connection accepted
                    status = Done;
                }
                else
                {
                    // Connection refused
                    status = Socket::get_error_status();
                }
            }
            else
            {
                // Failed to connect before timeout is over
                status = Socket::get_error_status();
            }
        }

        // Switch back to blocking mode
        set_blocking(true);

        return status;
    }
}


void TcpSocket::disconnect()
{
    // Close the socket
    close();

    // Reset the pending packet data
    pending_packet_ = PendingPacket();
}


Socket::Status TcpSocket::send(const void* data, std::size_t size){
    if (!is_blocking()) {
        LOG(util::Warning) << "Partial sends might not be handled properly";
    }
    std::size_t sent;
    return send(data, size, sent);
}


Socket::Status TcpSocket::send(const void* data, std::size_t size, std::size_t& sent)
{
    // Check the parameters
    if (!data || (size == 0))
    {
        LOG(util::Error) << "Cannot send data over the network (no data to send)";
        return Error;
    }

    // Loop until every byte has been sent
    int result = 0;
    for (sent = 0; sent < size; sent += result)
    {
        // Send a chunk of data
        result = ::send(get_handle(), static_cast<const char*>(data) + sent, static_cast<int>(size - sent), flags);

        // Check for errors
        if (result < 0)
        {
            Status status = Socket::get_error_status();

            if ((status == NotReady) && sent)
                return Partial;

            return status;
        }
    }

    return Done;
}


Socket::Status TcpSocket::receive(void* data, std::size_t size, std::size_t& received)
{
    // First clear the variables to fill
    received = 0;

    // Check the destination buffer
    if (!data)
    {
        LOG(util::Error) << "Cannot receive data from the network (the destination buffer is invalid)";
        return Error;
    }

    // Receive a chunk of bytes
    int sizeReceived = recv(get_handle(), static_cast<char*>(data), static_cast<int>(size), flags);

    // Check the number of bytes received
    if (sizeReceived > 0)
    {
        received = static_cast<std::size_t>(sizeReceived);
        return Done;
    }
    else if (sizeReceived == 0)
    {
        return Socket::Disconnected;
    }
    else
    {
        return Socket::get_error_status();
    }
}


Socket::Status TcpSocket::send(Packet& packet)
{
    // TCP is a stream protocol, it doesn't preserve messages boundaries.
    // This means that we have to send the packet size first, so that the
    // receiver knows the actual end of the packet in the data stream.

    // We allocate an extra memory block so that the size can be sent
    // together with the data in a single call. This may seem inefficient,
    // but it is actually required to avoid partial send, which could cause
    // data corruption on the receiving end.

    // Get the data to send from the packet
    std::size_t size = 0;
    const void* data = packet.on_send(size);

    // First convert the packet size to network byte order
    uint32 packetSize = htonl(static_cast<uint32>(size));

    // Allocate memory for the data block to send
    std::vector<char> blockToSend(sizeof(packetSize) + size);

    // Copy the packet size and data into the block to send
    std::memcpy(&blockToSend[0], &packetSize, sizeof(packetSize));
    if (size > 0)
        std::memcpy(&blockToSend[0] + sizeof(packetSize), data, size);

    // Send the data block
    std::size_t sent;
    Status status = send(&blockToSend[0] + packet.send_pos_, blockToSend.size() - packet.send_pos_, sent);

    // In the case of a partial send, record the location to resume from
    if (status == Partial)
    {
        packet.send_pos_ += sent;
    }
    else if (status == Done)
    {
        packet.send_pos_ = 0;
    }

    return status;
}


Socket::Status TcpSocket::receive(Packet& packet)
{
    // First clear the variables to fill
    packet.clear();

    // We start by getting the size of the incoming packet
    uint32 packetSize = 0;
    std::size_t received = 0;
    if (pending_packet_.SizeReceived < sizeof(pending_packet_.Size))
    {
        // Loop until we've received the entire size of the packet
        // (even a 4 byte variable may be received in more than one call)
        while (pending_packet_.SizeReceived < sizeof(pending_packet_.Size))
        {
            char* data = reinterpret_cast<char*>(&pending_packet_.Size) + pending_packet_.SizeReceived;
            Status status = receive(data, sizeof(pending_packet_.Size) - pending_packet_.SizeReceived, received);
            pending_packet_.SizeReceived += received;

            if (status != Done)
                return status;
        }

        // The packet size has been fully received
        packetSize = ntohl(pending_packet_.Size);
    }
    else
    {
        // The packet size has already been received in a previous call
        packetSize = ntohl(pending_packet_.Size);
    }

    // Loop until we receive all the packet data
    char buffer[1024];
    while (pending_packet_.Data.size() < packetSize)
    {
        // Receive a chunk of data
        std::size_t sizeToGet = std::min(static_cast<std::size_t>(packetSize - pending_packet_.Data.size()), sizeof(buffer));
        Status status = receive(buffer, sizeToGet, received);
        if (status != Done)
            return status;

        // Append it into the packet
        if (received > 0)
        {
            pending_packet_.Data.resize(pending_packet_.Data.size() + received);
            char* begin = &pending_packet_.Data[0] + pending_packet_.Data.size() - received;
            std::memcpy(begin, buffer, received);
        }
    }

    // We have received all the packet data: we can copy it to the user packet
    if (!pending_packet_.Data.empty())
        packet.on_receive(&pending_packet_.Data[0], pending_packet_.Data.size());

    // Clear the pending packet data
    pending_packet_ = PendingPacket();

    return Done;
}


TcpSocket::PendingPacket::PendingPacket() :
Size        (0),
SizeReceived(0),
Data        ()
{

}

} // namespace mahi
} // namespace com

//==============================================================================
// APAPTED FROM: SFML (https://www.sfml-dev.org/)
//==============================================================================
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2017 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//==============================================================================
