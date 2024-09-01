#!/bin/bash
#
# LICENSE: CC0
#

n=32
n2=`echo "$n / 2" | bc `

../../../bin/poms.ir \
  -s $n \
  -P $n2 \
  -o 1 \
  -F 0 \
  -V 0 \
  -C ../data/brutal-plum_poms.json > ./brutal-plum_simple.taccl

../../../bin/poms.ir \
  -s $n \
  -P $n2 \
  -o 1 \
  -F 1 \
  -C ../data/brutal-plum_poms.json > ./brutal-plum_freq.taccl
  
  
