#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <iomanip>

int main() {
    const char* portName = "/dev/ttyUSB0";
    int serialPort = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);

    if (serialPort < 0) {
        std::cerr << "Error " << errno << " opening " << portName << ": " << strerror(errno) << std::endl;
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    // Get the current configuration of the serial port.
    if (tcgetattr(serialPort, &tty) != 0) {
        std::cerr << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
        return -1;
    }

    // Set up the serial port configuration.
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    tty.c_cc[VTIME] = 5; // Wait for up to 0.5 seconds (5 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set the serial port attributes.
    if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
        std::cerr << "Error " << errno << " from tcsetattr: " << strerror(errno) << std::endl;
        return -1;
    }

    // Allocate memory for read buffer, set size according to your needs
    char read_buf [256];
    memset(&read_buf, '\0', sizeof(read_buf));

    // Read data from the serial port
    int num_bytes;
    while (true) {
        num_bytes = read(serialPort, &read_buf, sizeof(read_buf));
        if (num_bytes > 0) {
        
        std::cout << "\033[1;31mRead " << num_bytes << ":\033[0m\n";
        std::cout << "\033[1;32m00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15\033[0m\n";
        std::cout << "\033[1;32m-----------------------------------------------\033[0m\n";
        for(int i = 0; i < num_bytes; ++i) {
            // Cast each byte to unsigned int and print in hexadecimal
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)read_buf[i] << " ";
            if( (i+1) % 16 == 0 )
            {
               std::cout << '\n';
            }
        }
            std::cout << "\n\n";
            std::cout.flush();
        } else {
            // No data was read, but we don't treat it as an error
            //std::cout << std::endl; // Break the line if timeout
        }
        usleep(300000); // Sleep for 10 ms to avoid hogging CPU, adjust as needed
    }

    close(serialPort);
    return 0;
}

