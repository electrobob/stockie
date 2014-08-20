/*
 */


#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "serial.h"
#include "../nrf24.h"
#include "../spi.h"


// TX = PC3, RX = PC2

uart debug(&USARTC0, 57600); 
ISR (USARTC0_RXC_vect){ debug.rxInterrupt(); }
ISR (USARTC0_DRE_vect){ debug.txInterrupt(); }



/*
nRF 0
Miso = PC6
Mosi = PC5
clk  = PC7
csn  = PC4
irq	 = PC1
ce	 = PC0
*/

spiDriver nrfSpi(&SPIC, &PORTC, 5, 6, 7);
nrf24 nrf0(&nrfSpi, &PORTC, 4, &PORTC, 1, &PORTC, 0);

ISR (PORTC_INT0_vect) {nrf0.pinInterrupt();}
ISR (SPIC_INT_vect) { if(nrfSpi.interrupt()) nrf0.spiInterrupt();}



int main(void)
{

	//Config external 16 MHz clock
	OSC.XOSCCTRL = 0x17; 
	OSC.CTRL = OSC_XOSCEN_bm;
	while (!(OSC.STATUS & OSC_XOSCRDY_bm));
	
	CCP = CCP_IOREG_gc;
	CLK.PSCTRL = 0;
	
	CCP = CCP_IOREG_gc;
	CLK.CTRL = CLK_SCLKSEL_XOSC_gc;

	
    //Enable all interrupts
    PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	sei();


    debug.sendStringPgm(PSTR("\n\n\nBasic example of half-duplex communication\n"));
	
	#define PAYLOAD_SIZE 1

	nrf0.primaryRx();

	nrf0.setRegister(NRF_RX_PW_P0, PAYLOAD_SIZE);
	nrf0.setRegister(NRF_RX_PW_P1, PAYLOAD_SIZE);
	nrf0.setRegister(NRF_SETUP_RETR, 0x33);
	
	
	uint8_t rx_data[PAYLOAD_SIZE + 1];
	uint8_t tx_data[PAYLOAD_SIZE + 1];

	uint8_t tx_bytes = 0;



    while(1)
    {

		nrf0.primaryRx();		
		nrf0.reciveData(rx_data,PAYLOAD_SIZE);
		
		while(nrf0.getState() != rx_idle);
		_delay_ms(10);

		nrf0.primaryTx();
		
		nrf0.sendData(rx_data, PAYLOAD_SIZE);	
		while(nrf0.getState() != tx_idle);
	_delay_ms(10);

 
		
	}
		



    return 0;
}
/*
void printAllRegisters(nrf24 &nrf)
{
	printRegister(PSTR("Config"), NRF_CONFIG, 1, nrf);
	printRegister(PSTR("En AA"), NRF_EN_AA, 1, nrf);
	printRegister(PSTR("En rx pipe"), NRF_EN_RXADDR, 1, nrf);
	printRegister(PSTR("Aw"), NRF_SETUP_AW, 1, nrf);
	printRegister(PSTR("Re trans"), NRF_SETUP_RETR, 1, nrf);
	printRegister(PSTR("RH ch"), NRF_RF_CH, 1, nrf);
	printRegister(PSTR("RF setup"), NRF_RF_SETUP, 1, nrf);
	printRegister(PSTR("Status"), NRF_STATUS, 1, nrf);
	printRegister(PSTR("Obs tx"), NRF_OBSERVE_TX, 1, nrf);
	printRegister(PSTR("RPD"), NRF_RPD, 1, nrf);
	printRegister(PSTR("RX0 addr"), NRF_RX_ADDR_P0, 5, nrf);
	printRegister(PSTR("RX1 addr"), NRF_RX_ADDR_P1, 5, nrf);
	printRegister(PSTR("RX2 addr"), NRF_RX_ADDR_P2, 1, nrf);
	printRegister(PSTR("RX3 addr"), NRF_RX_ADDR_P3, 1, nrf);
	printRegister(PSTR("RX4 addr"), NRF_RX_ADDR_P4, 1, nrf);
	printRegister(PSTR("RX5 addr"), NRF_RX_ADDR_P5, 1, nrf);
	printRegister(PSTR("TX addr"), NRF_TX_ADDR, 5, nrf);
	
	printRegister(PSTR("RX0 pl"), NRF_RX_PW_P0, 1, nrf);
	printRegister(PSTR("RX1 pl"), NRF_RX_PW_P1, 1, nrf);
	printRegister(PSTR("RX2 pl"), NRF_RX_PW_P2, 1, nrf);
	printRegister(PSTR("RX3 pl"), NRF_RX_PW_P3, 1, nrf);
	printRegister(PSTR("RX4 pl"), NRF_RX_PW_P4, 1, nrf);
	printRegister(PSTR("RX5 pl"), NRF_RX_PW_P5, 1, nrf);
	
	printRegister(PSTR("FIFO"), NRF_FIFO_STATUS, 1, nrf);
	printRegister(PSTR("Dyn pl"), NRF_DYNPD, 1, nrf);
	printRegister(PSTR("Feature"), NRF_FEATURE, 1, nrf);
	
}

void printRegister(const char *str, uint8_t reg, uint8_t len, nrf24 &nrf)
{
	uint8_t data[5];
	nrf.getRegister(reg, data, len);
	
	debug.sendStringPgm(str);
	debug.sendString(": ");
	
	for (uint8_t i = 0; i < len; i++)
		debug.sendHex(data[i]);
		
	debug.sendChar('\n');
}*/ 
