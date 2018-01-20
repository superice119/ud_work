#!/bin/sh

cd `dirname $0`

while true
do
	./poly_pkt_fwd
	killall poly_pkt_fwd 2> /dev/null
done

exit 0
