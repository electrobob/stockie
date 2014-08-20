
#ifndef INCLUDE_SPI
#define INCLUDE_SPI

#include <avr/io.h>



class spiDriver
{
public:
    spiDriver(SPI_t* const spi, PORT_t* const spiPort, const uint8_t mosiPin, const uint8_t misoPin, const uint8_t clkPin);
    void setSpeed(const SPI_PRESCALER_enum prescaler, const bool doubleSpeed);
    void setMode(const SPI_MODE_enum mode);
    void setMasterMode(const bool master);
    void enable() {_spi->CTRL |= SPI_ENABLE_bm;}
    void disable() {_spi->CTRL &= ~SPI_ENABLE_bm;}

    void transmit(uint8_t* const data, const uint8_t len, PORT_t* const ssPort, const uint8_t ssPin);
    uint8_t transmit(const uint8_t data, PORT_t* const ssPort, const uint8_t ssPin);

    bool isTransmitting() {return _isTransmitting;}
    void flush();

    bool interrupt()
    {
        _dataPtr[_bytesSent++] = _spi->DATA;
        if(_dataLen == _bytesSent)
        {
            _isTransmitting = false;
            _spi->INTCTRL = SPI_INTLVL_OFF_gc;
		    _ssPort->OUTSET = _ssPinBm;
            return true;
        }
        else
        {
            _spi->DATA = _dataPtr[_bytesSent];
            return false;
        }
    }



private:
    SPI_t* const _spi;
    PORT_t* _ssPort;
    uint8_t _ssPinBm;

    uint8_t* _dataPtr;
    volatile bool _isTransmitting;
	uint8_t _bytesSent;
    uint8_t _dataLen;
};






#endif
