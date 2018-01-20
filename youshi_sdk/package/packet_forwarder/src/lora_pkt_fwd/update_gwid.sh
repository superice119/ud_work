#!/bin/sh

# This script is a helper to update the Gateway_ID field of given
# JSON configuration file, as a EUI-64 address generated from the 48-bits MAC
# address of the device it is run from.
#
# Usage examples:
#       ./update_gwid.sh ./local_conf.json

iot_sk_update_gwid() {
    # get gateway ID from its MAC address to generate an EUI-64 address
    GWID_MIDFIX="FFFF"
    GWID_BEGIN=$(cat /sys/class/net/eth0/address | awk -F\: '{print $1$2$3}')
    GWID_END=$(cat /sys/class/net/eth0/address | awk -F\: '{print $4$5$6}')

    # replace last 8 digits of default gateway ID by actual GWID, in given JSON configuration file
    sed -i "s/AA555A0000000101/${GWID_BEGIN}${GWID_MIDFIX}${GWID_END}/g" $1

    echo "Gateway_ID set to "$GWID_BEGIN$GWID_MIDFIX$GWID_END" in file "$1
}

if [ $# -ne 1 ]
then
    echo "Usage: $0 [filename]"
    echo "  filename: Path to JSON file containing Gateway_ID for packet forwarder"
    exit 1
fi 

iot_sk_update_gwid $1

exit 0
