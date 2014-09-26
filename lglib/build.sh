#!/bin/sh

make \
	CC=/opt/microchip/xc16/v1.21/bin/xc16-gcc \
	AS=/opt/microchip/xc16/v1.21/bin/xc16-as \
	AR=/opt/microchip/xc16/v1.21/bin/xc16-ar \
	RM="rm -f" \
	OPT=-O1 \
	$@
