#!/bin/sh

cd `dirname $0`

while true
do
	./lora_pkt_fwd
	killall lora_pkt_fwd 2> /dev/null
done

exit 0
