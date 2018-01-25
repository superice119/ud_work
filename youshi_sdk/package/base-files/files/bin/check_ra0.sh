#!/bin/sh
# Copyright (C) 2006-2011 OpenWrt.org

#cd / 

while true
do
	ifname=`brctl show | grep ra`

	if [ -z ${ifname} ]; then
	    echo " add ${ifname} "
		#ubus call network.interface.lan add_device '{"name":"ra0"}'
		`brctl addif br-lan ra0`
	fi

done

exit 0

