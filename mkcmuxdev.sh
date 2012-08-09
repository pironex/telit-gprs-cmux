#!/bin/sh

if [ -e /dev/ttygsm1 ] ; then
	exit 0 ;
fi

MAJOR=`cat /proc/devices |grep gsmtty | awk '{print $1}`
if [[ $MAJOR == "" ]] ; then
	exit 1 ;
fi

for i in `seq 1 4`; do
	mknod /dev/ttygsm$i c $MAJOR $i
done
