/*
software_uart

 copyright (c) Bogdan Raducanu, 2014
 Created: July 2014
  Author: Bogdan Raducanu
  bogdan@electrobob.com

	www.electrobob.com 
	

Released under GPLv3.
Please refer to LICENSE file for licensing information.	
*/

#include <avr/io.h>
#include "soft_usart_tx.h"
#include <util/delay.h>

//damn simple software uart
void send_char(char data)
{
	//send start
	UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	
	
	//1 bit
	if(data & 1) UTXPORT |= (1<<UTXPIN); else UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	//2 bit
	if(data & 2) UTXPORT |= (1<<UTXPIN); else UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	//3 bit
	if(data & 4) UTXPORT |= (1<<UTXPIN); else UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	//5 bit
	if(data & 8) UTXPORT |= (1<<UTXPIN); else UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	//6 bit
	if(data & 16) UTXPORT |= (1<<UTXPIN); else UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	//6 bit
	if(data & 32) UTXPORT |= (1<<UTXPIN); else UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	//7 bit
	if(data & 64) UTXPORT |= (1<<UTXPIN); else UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	//8 bit
	if(data & 128) UTXPORT |= (1<<UTXPIN); else UTXPORT &= ~(1<<UTXPIN);
	_delay_us(833);
	
	//stop bit
	UTXPORT |= (1<<UTXPIN);
	_delay_us(833);
}

void send_string(char *str){
	while (*str != '\0')
	{
		send_char(*str);
		//putchar_usartc0(*str);
		str++;
	}

}