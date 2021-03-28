#include <string>


int MacOpenComport(std::string port_name, int baud_rate, const char * mode);

int MacSendBuf(std::string port_name, unsigned char *data, int size);

int MacPollComport(std::string port_name, unsigned char *data, int size);

void MacflushRX(std::string port_name);

void MacflushTX(std::string port_name);

void MacflushRXTX(std::string port_name);

void MacCloseComport(std::string port_name);