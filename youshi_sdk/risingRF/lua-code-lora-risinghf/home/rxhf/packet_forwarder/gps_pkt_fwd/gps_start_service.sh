#!/bin/sh

cd `dirname $0`

while true
do
	./gps_pkt_fwd
	killall gps_pkt_fwd 2> /dev/null
done

exit 0
