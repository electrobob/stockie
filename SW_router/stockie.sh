#!/bin/sh
#
# stockie.sh
# copyright (c) Bogdan Raducanu, 2014
# Created: AUG 2014
#  Author: Bogdan Raducanu
#  bogdan@electrobob.com
#
#	www.electrobob.com 
#	
#
#Released under GPLv3.
#Please refer to LICENSE file for licensing information.	
#
#set file to run at startup




user_name="my_super_secret_username"
pass_word="my_super_secret_password"

echo "Starting stockie service" > /dev/ttyATH0

while true
do
	temp=$(grep -m 1 Stockie /dev/ttyUSB0)
	echo "Received $temp" > /dev/ttyATH0

	#separate the received in pieces
	var1=$(echo $temp | cut -f1 -d" ")
	command=$(echo $temp | cut -f2 -d" ")
	stockie_id=$(echo $temp | cut -f3 -d" ")
	battery=$(echo $temp | cut -f4 -d" ")

	#process the commands
	if [[ "$command" == "toggle" ]]
	then
	echo "we are togling" > /dev/ttyATH0
 	link="www.electrobob.com/scgi-bin/item_toggle.php?username=$user_name&key=$pass_word&id=$stockie_id"
	http_return=$(curl $link)
	echo "HTTP response: $http_return" > /dev/ttyATH0
	echo $link > /dev/ttyATH0
	echo $http_return > /dev/ttyUSB0
	fi

done