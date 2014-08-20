/*
Stockie Gateway

 copyright (c) Bogdan Raducanu, 2014
 Created: August 2014
  Author: Bogdan Raducanu
  bogdan@electrobob.com

	www.electrobob.com 
	

Released under GPLv3.
Please refer to LICENSE file for licensing information.	
*/


#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>


#include "ws2812.h"
#include "gammacorrection.h"


#include "serial.h"
#include "nrf24.h"
#include "spi.h"



#define NUMLEDS     1     // number of leds on strip

//stockie stuff
uint32_t stockie_id;
uint16_t stockie_batt;
enum stockie_mode {wait_nrf_rx, wait_router_rx, wait_nrf_tx, wait_router_tx};
char router_rx_buf[64];
char router_rx_buf_ptr=0;
char router_data_ready=0;

//LED set
#define SET_LED(red,grn,blu) {MyData[0]=grn; MyData[1]=red; MyData[2]=blu;	 WS_out(MyData, sizeof(MyData), GammaTable); }

static uint8_t MyData[NUMLEDS*3];
uint8_t LED_R, LED_G, LED_B;
static const uint8_t PROGMEM GammaTable[] = GCN_TABLE7TO8;

uart debug(&USARTD0, 115200);
uart router_serial(&USARTE0, 115200);
ISR (USARTD0_RXC_vect){ debug.rxInterrupt(); }
ISR (USARTD0_DRE_vect){ debug.txInterrupt(); }
ISR (USARTE0_RXC_vect){ router_serial.rxInterrupt(); }
ISR (USARTE0_DRE_vect){ router_serial.txInterrupt(); }

/*
nRF 0
Miso = PC6
Mosi = PC5
clk  = PC7
csn  = PC4
irq	 = PC1
ce	 = PC2
*/
spiDriver nrfSpi(&SPIC, &PORTC, 5, 6, 7);
nrf24 nrf0(&nrfSpi, &PORTC, 4, &PORTC, 1, &PORTC, 2);

uint8_t auto_transmit=0;
uint32_t pack_cntr = 0;
uint32_t pack_cntr_rx = 0;
uint32_t pack_cntr_rx_old=0;
uint32_t pack_cntr_rx_missed=0;
char out_str[16];


ISR (PORTC_INT0_vect) {nrf0.pinInterrupt();}
ISR (SPIC_INT_vect) { if(nrfSpi.interrupt()) nrf0.spiInterrupt();}

void printRegister(const char *str, uint8_t reg, uint8_t len, nrf24 &nrf)
{
	uint8_t data[5];
	data[0]=nrf.getRegister(reg);
	debug.sendStringPgm(str);
	debug.sendString(": ");
	for (uint8_t i = 0; i < len; i++)
	debug.sendHex(data[i]);
	debug.sendChar('\n');
}

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




int main(void)
{

	
	CCP = CCP_IOREG_gc; //some special stuff inside can be modified through this security stuff
	OSC.CTRL=OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP=CCP_IOREG_gc;
	CLK.CTRL=0x01;
	
	//Enable all interrupts
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	//PMIC.CTRL =  PMIC_MEDLVLEN_bm ;
	sei();

	
	//WS_init(); disable for now

	debug.sendStringPgm(PSTR("\n\n\nStockie debug interface\n"));
	router_serial.sendStringPgm(PSTR("\n\n\nStockie router interface ###to be removed####\n"));
	
	#define PAYLOAD_SIZE 12
	_delay_ms(100);
	//printAllRegisters(nrf0);
	//_delay_ms(5000); //wait for a while
	nrf0.primaryRx();
	

	nrf0.setRegister(NRF_RX_PW_P0, PAYLOAD_SIZE);
	//nrf0.setRegister(NRF_RX_PW_P1, PAYLOAD_SIZE);
	nrf0.setRegister(NRF_SETUP_RETR, 0x3F); //15 retries
	//nrf0.setRegister(NRF_RF_CH, 2);
	
	
	uint8_t rx_data[PAYLOAD_SIZE + 1];
	uint8_t tx_data[PAYLOAD_SIZE + 1];
	uint8_t tx_data_auto_init[PAYLOAD_SIZE + 1]="bob 1234";
	uint8_t tx_data_auto[PAYLOAD_SIZE + 1];
	uint8_t tx_bytes = 0;


	nrf0.primaryRx();

	auto_transmit = 0;

	
	nrf0.primaryRx();
	nrf0.reciveData(rx_data,PAYLOAD_SIZE);
	//LED_R=0;

	//stockie_mode = wait_nrf_rx;
	while(1)
	{
		//dump what comes from the router in a buffer(yeah i know usart already has buffer, but cannot read without erasing, need lib change)
		if(router_serial.dataAvailable()){
			router_rx_buf[router_rx_buf_ptr]=router_serial.getChar();
			if(router_rx_buf[router_rx_buf_ptr]=='\n') router_data_ready = 1;
			router_rx_buf_ptr++;
		}
		
		if(router_data_ready){
		debug.sendStringPgm(PSTR("Router RX: "));
		for(uint8_t i = 0; i < router_rx_buf_ptr; i++)
		debug.sendChar(router_rx_buf[i]); //first data is garbage
		router_data_ready=0;
		router_rx_buf_ptr=0;
		
		//check if ok, then send ok to tag
		if((router_rx_buf[8]=='o') && (router_rx_buf[9]=='k'))
		{
		tx_data_auto[1]='s';
		tx_data_auto[2]='t';
		tx_data_auto[3]='k';
		tx_data_auto[4]='0';
		tx_data_auto[5]=router_rx_buf[11];	
		}
		
		debug.sendStringPgm(PSTR("NRF TX: "));
		for(uint8_t i = 1; i < PAYLOAD_SIZE+1; i++)
		debug.sendChar(tx_data_auto[i]); //first data is garbage
		//tx_bytes = 0;
		nrf0.primaryTx();
		
		nrf0.sendData(tx_data_auto, PAYLOAD_SIZE);
		while(nrf0.getState() != tx_idle);
		
					//restart the radio as receiver
					nrf0.primaryRx();
					nrf0.reciveData(rx_data,PAYLOAD_SIZE);
		
		}
		//do some stuff while waiting for radio
		LED_R++;
		if(LED_R>64) LED_R=0;
		//SET_LED(LED_R,LED_G, LED_B);

		//if radio data is available
		if(nrf0.getState() == rx_idle){ //wait for rx idle
			//Dump data on debug
			debug.sendStringPgm(PSTR("NRF RX: "));
			for(uint8_t i = 0; i < PAYLOAD_SIZE; i++)
			debug.sendChar(rx_data[i+1]); //first data is garbage
			debug.sendChar('\n');
			if((rx_data[1]=='s') &&  (rx_data[2]=='t') && (rx_data[3]=='k')) //if stockie header received:
			{
				//let the serial know
				debug.sendStringPgm(PSTR("Stockie packet"));
				
				//convert packet, send to router and dump do debug.
				router_serial.sendStringPgm(PSTR("\nStockie "));
				debug.sendStringPgm(PSTR("\n Router TX: Stockie "));
				if(rx_data[4]=='t'){ //if command is to toggle
					router_serial.sendString("toggle ");
					debug.sendString("toggle ");
					stockie_id = rx_data[5]<<24;
					stockie_id += rx_data[6]<<16;
					stockie_id += rx_data[7]<<8;
					stockie_id += rx_data[8];
					itoa(stockie_id, out_str, 10);
					router_serial.sendString(out_str);
					router_serial.sendChar(' ');
					debug.sendString(out_str);
					debug.sendChar(' ');
					stockie_batt = rx_data[9]<<8;
					stockie_batt += rx_data[10];
					itoa(stockie_batt, out_str, 10);
					router_serial.sendString(out_str);
					router_serial.sendChar('\n');
					debug.sendString(out_str);
					debug.sendChar('\n');
				}
				if(rx_data[4]=='c'){
					router_serial.sendString("configure ");
					debug.sendString("configure ");
					stockie_id = rx_data[5]<<24;
					stockie_id += rx_data[6]<<16;
					stockie_id += rx_data[7]<<8;
					stockie_id += rx_data[8];
					itoa(stockie_id, out_str, 10);
					router_serial.sendString(out_str);
					router_serial.sendChar(' ');
					debug.sendString(out_str);
					debug.sendChar(' ');
					stockie_batt = rx_data[9]<<8;
					stockie_batt += rx_data[10];
					itoa(stockie_batt, out_str, 10);
					router_serial.sendString(out_str);
					router_serial.sendChar('\n');
					debug.sendString(out_str);
					debug.sendChar('\n');
				}
			}
			//restart the radio as receiver
			nrf0.primaryRx();
			nrf0.reciveData(rx_data,PAYLOAD_SIZE);
		}
	}

	return 0;
}

