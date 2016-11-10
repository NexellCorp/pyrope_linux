#!/bin/sh
cmp -s ./lib/libnxcinema.so ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/lib/libnxcinema.so
if [ $? -ne 0 ]; then
    echo ">>> Update libnxcinema.so"
    cp -av ./lib/libnxcinema.so ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/lib/libnxcinema.so
fi

cmp -s ./src/include/NX_TMSClient.h ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/include/NX_TMSClient.h
if [ $? -ne 0 ]; then
    echo ">>> Update NX_TMSClient.h"
    cp -av ./src/include/NX_TMSClient.h ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/include/NX_TMSClient.h
fi

cmp -s ./src/include/NX_TMSCommand.h ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/include/NX_TMSCommand.h
if [ $? -ne 0 ]; then
    echo ">>> Update NX_TMSCommand.h"
    cp -av ./src/include/NX_TMSCommand.h ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/include/NX_TMSCommand.h
fi

