#!/bin/sh
NX_DIR_WORKING=$(pwd)
NX_DIR_SCRIPT=$(cd "$(dirname "$0")" && pwd)
NX_DIR_RESULT=$NX_DIR_SCRIPT/result

if [ -f $NX_DIR_SCRIPT/NxCinemaDiagnostics/NxCinemaDiagnostics ]; then
	cmp -s $NX_DIR_SCRIPT/NxCinemaDiagnostics/NxCinemaDiagnostics $NX_DIR_RESULT/bin/NxCinemaDiagnostics
	if [ $? -ne 0 ]; then
		echo ">>> Update NxCinemaDiagnostics"
		cp -av $NX_DIR_SCRIPT/NxCinemaDiagnostics/NxCinemaDiagnostics $NX_DIR_RESULT/bin/NxCinemaDiagnostics
	fi
fi

if [ -f $NX_DIR_SCRIPT/NxCinemaDiagnosticsPanel/Release/NxCinemaDiagnosticsPanel.exe ]; then
	cmp -s $NX_DIR_SCRIPT/NxCinemaDiagnosticsPanel/Release/NxCinemaDiagnosticsPanel.exe $NX_DIR_RESULT/NxCinemaDiagnosticsPanel.exe
	if [ $? -ne 0 ]; then
		echo ">>> Update NxCinemaDiagnosticsPanel"
		cp -av $NX_DIR_SCRIPT/NxCinemaDiagnosticsPanel/Release/NxCinemaDiagnosticsPanel.exe $NX_DIR_RESULT/NxCinemaDiagnosticsPanel.exe
	fi
fi

cd $NX_DIR_SCRIPT/result
zip -r $NX_DIR_SCRIPT/NxCinemaDiagnosticsPanel.zip *

cd $NX_DIR_WORKING
