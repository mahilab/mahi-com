target_sources(com
    PRIVATE
    Http.cpp
    IpAddress.cpp
    MelNet.cpp
    MelShare.cpp
    Packet.cpp
    SerialPort.cpp
    SharedMemory.cpp
    Socket.cpp
    SocketSelector.cpp
    TcpListener.cpp
    TcpSocket.cpp
    UdpSocket.cpp
    Detail/rs232.cpp
    Detail/rs232.hpp
    # Detail/MacSerial.cpp
    # Detail/MacSerial.hpp
)