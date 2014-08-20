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


#ifndef SOFT_USART_TX_H_
#define SOFT_USART_TX_H_


#define UTXPORT PORTA
#define UTXPIN PINA2
void send_char(char data);
void send_string(char *str);

#endif /* SOFT_USART_TX_H_ */