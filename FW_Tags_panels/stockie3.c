/*
 stockie3_10_v1.cpp
 copyright (c) Bogdan Raducanu, 2014
 Created: July 2014
  Author: Bogdan Raducanu
  bogdan@electrobob.com

	www.electrobob.com 
	

Released under GPLv3.
Please refer to LICENSE file for licensing information.	
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "soft_usart_tx.h"
#include "nrf24l01.h"
#include "nrf24l01registers.h"

#define DEBUGENABLED 0

//HW ID - to be changed later
#define HW1 0 //MSB
#define HW2 0
#define HW3 3 //LSB

//Button definitions - order is of no importance, they will be dynamically assigned.
#define B1_PORT PINC
#define B1_PIN PINC4
#define B2_PORT PIND
#define B2_PIN PIND0
#define B3_PORT PINC
#define B3_PIN PINC5
#define B4_PORT PINC
#define B4_PIN PINC1
#define B5_PORT PINC
#define B5_PIN PINC2
#define B6_PORT PINC
#define B6_PIN PINC3
#define B7_PORT PIND
#define B7_PIN PIND1
#define B8_PORT PIND
#define B8_PIN PIND2
#define B9_PORT PIND
#define B9_PIN PIND3
#define B10_PORT PIND
#define B10_PIN PIND4

//role definitions
#define ROLETX 1
#define ROLERX 0

//role button/pin
#define BUTTONROLE_DDR DDRC
#define BUTTONROLE_PORT PORTC
#define BUTTONROLE_PIN PINC
#define BUTTONROLE PC0
//send button
#define BUTTONSEND_DDR DDRC
#define BUTTONSEND_PORT PORTC
#define BUTTONSEND_PIN PINC
#define BUTTONSEND PC1
//output led
#define LEDOUT_DDR DDRC
#define LEDOUT_PORT PORTC
#define LEDOUT PC5
#define LEDOUTPAUSE 300

//led usage
#define LEDR_ON PORTB|=(1<<PB1)
#define LEDR_OFF PORTB&=~(1<<PB1)
#define LEDG_ON PORTB|=(1<<PB2)
#define LEDG_OFF PORTB&=~(1<<PB2)

/*
nRF 0
Miso = PB4
Mosi = PB3
clk  = PB5
csn  = PB6
irq	 = PB7
ce	 = PB0
VDD = PA3

*/

char buff[100];

uint8_t auto_transmit=0;
char out_str[16];


uint8_t buffer[32];
char button = 0;
char delay_counter=0;

char button_read(void){
	char button=0;
	if((B1_PORT & (1<<B1_PIN))==0) button=1;
	if((B2_PORT & (1<<B2_PIN))==0) button=2;
	if((B3_PORT & (1<<B3_PIN))==0) button=3;
	if((B4_PORT & (1<<B4_PIN))==0) button=4;
	if((B5_PORT & (1<<B5_PIN))==0) button=5;
	if((B6_PORT & (1<<B6_PIN))==0) button=6;
	if((B7_PORT & (1<<B7_PIN))==0) button=7;
	if((B8_PORT & (1<<B8_PIN))==0) button=8;
	if((B9_PORT & (1<<B9_PIN))==0) button=9;
	if((B10_PORT & (1<<B10_PIN))==0) button=10;

	return button;
}
int main(void)
{
	CLKPR=0x80;
	CLKPR=0x01; //change osc to 4 mhz
	
	//_delay_ms(1000);

	//PORTA all in, PU
	PORTA=0xFF;
	DDRA=0xFF;

	//PORTB 7,4, in, rest out
	PORTB=0x00;
	DDRB=0x6F;

	//PORTC all in, PU
	PORTC=0xFF;
	DDRC=0xFF;

	//PORT D all in,PU
	PORTD=0xFF;
	DDRD=0xFF;

	// Timer 0 disabled
	TCCR0A=0x00;
	TCNT0=0x00;
	OCR0A=0x00;
	OCR0B=0x00;


	// Timer/Counter 1 initialization - all off
	TCCR1A=0x00;
	TCCR1B=0x00;
	TCNT1H=0x00;
	TCNT1L=0x00;
	ICR1H=0x00;
	ICR1L=0x00;
	OCR1AH=0x00;
	OCR1AL=0x00;
	OCR1BH=0x00;
	OCR1BL=0x00;

	// Timer/Counter 1 initialization


	// External Interrupt(s) initialization
	EICRA=0x00;
	EIMSK=0x00;
	PCMSK0=0x80;
	PCICR=0x01;
	PCIFR=0x01;

	// Timer/Counter 0 Interrupt(s) initialization
	TIMSK0=0x01;

	// Timer/Counter 1 Interrupt(s) initialization
	TIMSK1=0x00;

	// comparator off
	ACSR=0x80;
	ADCSRB=0x00;
	DIDR1=0x00;

	// adc off
	ADCSRA=0x00;

	// TWI off
	TWCR=0x00;

	uint8_t txrxrole = 1; // 1 transmitter 0 receiver
	uint8_t i = 0;

	//nrf24l01 variables
	uint8_t bufferout[NRF24L01_PAYLOAD];
	uint8_t bufferin[NRF24L01_PAYLOAD];

	#if DEBUGENABLED == 1
	send_char('\n');
	send_char('0');
	#endif
	



	LEDR_ON;
	_delay_ms(100);
	LEDR_OFF;

	nrf24l01_init();
	_delay_ms(100);
	
	#if DEBUGENABLED == 1
	send_char('2');
	#endif

	uint8_t sendpipe = 0;
	uint8_t addrtx0[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP0;

	char data;

	while(1)
	{
		send_char('.');
		_delay_ms(50);
		//check which button is pressed
		button=button_read();
		if(button){
			send_char('\n');
			bufferout[0]='s'; //header
			bufferout[1]='t'; //header
			bufferout[2]='k';
			bufferout[3]='t';
			bufferout[4] = HW1; //#B unique HW
			bufferout[5] = HW2;
			bufferout[6] = HW3;
			bufferout[7] = button; //1B button addr
			bufferout[8] = 1; //2B battery status
			bufferout[9] = 100; //2B battery status
			
			#if DEBUGENABLED == 1
			char pipebuffer[5];
			send_string("sending data, on pipe ");
			itoa(sendpipe, pipebuffer, 10);
			send_string(pipebuffer);
			send_string("... ");
			#endif
			//OCR1AL=255;
			nrf24l01_settxaddr(addrtx0);


			//write buffer
			uint8_t writeret = nrf24l01_write(bufferout);
			_delay_ms(10);
			//OCR1AL=0;
			#if DEBUGENABLED == 1
			if(writeret == 1)
			send_string("ok\r\n");
			else
			send_string("failed\r\n");
			#endif
			
			#if DEBUGENABLED == 1
			send_string("waiting for data\r\n");
			#endif
			
			//read buffer
			uint8_t pipe = 0;
			delay_counter=0;
			while((delay_counter<100) && !(nrf24l01_readready(&pipe))) //wait max 1 second for the response
			{
				delay_counter++;
				_delay_ms(10);
			}
			
			#if DEBUGENABLED == 1
			if(delay_counter==100)
			send_string("\r\ntimed out ");
			#endif
			
			
			//flush the buff
			for(uint8_t i=0; i<12; i++) bufferin[i]=0;
			if(nrf24l01_readready(&pipe)) { //if data is ready

				nrf24l01_read(bufferin);
				
				#if DEBUGENABLED == 1
				send_string("\r\nreceived ");
				for(uint8_t i = 0; i<12; i++)
				send_char(bufferin[i]);
				#endif
				
				//check if successful
				if(bufferin[0]=='s' && bufferin[1]=='t' && bufferin[2]=='k'){
					if(bufferin[4]=='1')
					{ //blink pattern for on stock
						LEDG_ON;
						_delay_ms(100);
						LEDG_OFF;
						_delay_ms(100);
						LEDG_ON;
						_delay_ms(100);
						LEDG_OFF;
						_delay_ms(100);
						LEDG_ON;
						_delay_ms(100);
						LEDG_OFF;
						_delay_ms(100);
						
					} else
					{//blink pattern for NOT on stock
						LEDR_ON;
						_delay_ms(100);
						LEDR_OFF;
						_delay_ms(100);
						LEDR_ON;
						_delay_ms(100);
						LEDR_OFF;
						_delay_ms(100);
						LEDR_ON;
						_delay_ms(100);
						LEDR_OFF;
						_delay_ms(100);
					}
				}
				OCR1AL=0;
				OCR1BL=0;
			}

		}
	}
	return 0;
}