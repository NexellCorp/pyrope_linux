#!/bin/sh
cmp -s ./lib/libnxcinema.so ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/lib/libnxcinema.so
if [ $? -ne 0 ]; then
    echo ">>> Update libnxcinema.so"
    cp -av ./lib/libnxcinema.so ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/lib/libnxcinema.so
fi

cmp -s ./src/include/NX_IPCClient.h ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/include/NX_IPCClient.h
if [ $? -ne 0 ]; then
    echo ">>> Update NX_IPCClient.h"
    cp -av ./src/include/NX_IPCClient.h ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/include/NX_IPCClient.h
fi

cmp -s ./src/include/NX_CinemaCommand.h ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/include/NX_CinemaCommand.h
if [ $? -ne 0 ]; then
    echo ">>> Update NX_CinemaCommand.h"
    cp -av ./src/include/NX_CinemaCommand.h ./apps/CinemaSolution/CinemaControlPanel/src/main/jni/include/NX_CinemaCommand.h
fi

