#!/bin/bash
g++ -c -arch i386 -arch x86_64 -O3 -fno-tree-pre -fno-strict-aliasing -DNDEBUG -msse3 -mssse3 -I/usr/include/nite -I/usr/include/ni -DXN_SSE -o Release/main.o ../SingleControl/main.cpp
g++ -o ./SingleControl ./Release/main.o ./Release/signal_catch.o ./Release/kbhit.o -arch i386 -arch x86_64  -L../Bin -lOpenNI -lXnVNite

