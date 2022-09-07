#ifndef __spi_h__
#define __spi_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using std::string;
class SPIClass {
public:
    SPIClass();
    void begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);
};

#endif