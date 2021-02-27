// #include <MEL/Communications/SerialPort.hpp>
// #include <MEL/Core/Console.hpp>
#include <Mahi/Util.hpp>
#include <Mahi/Com.hpp>

using namespace mahi::util;
using namespace mahi::com;

int main() {

    SerialPort comm4;
    SerialPort comm5;

    comm4.open(3, 9600); // 3 = COM4
    comm5.open(4, 9600); // 4 = COM5

    unsigned char send[5] = "abcd";

    comm4.send_data(send, 5);
    prompt("Press ENTER to receive");

    unsigned char recv[5];
    comm5.receive_data(recv, 5);
    for (std::size_t i = 0; i < 5; ++i)
        std::cout << recv[i] << std::endl;

    return 0;
}
