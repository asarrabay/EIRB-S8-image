#!/bin/bash

for i in `seq 0 8`;
do
  ./pipeline ../data/test-0$i.ppm res0$i.ppm
done
