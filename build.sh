#!/bin/sh
set -x
rm -rf obj/*
__MAC__=1 idamake.pl -j 10 2>&1
if [ $? -eq 0 ]
then
        cp ../../bin/plugins/desquirr.pmc /Applications/IDA\ Starter\ 6.95/idabin/plugins/
else
        echo "COMPILE ERROR!"
fi

