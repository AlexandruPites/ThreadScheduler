#!/bin/bash
make clean
make
cp libscheduler.so ../checker-lin
make clean
cd ../checker-lin
make -f Makefile.checker clean
make -f Makefile.checker