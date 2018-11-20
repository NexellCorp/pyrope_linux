#!/bin/sh
NX_DIR_WORKING=$(pwd)
NX_DIR_SCRIPT=$(cd "$(dirname "$0")" && pwd)
NX_DIR_RESULT=$NX_DIR_SCRIPT/apps/CinemaDiagnostics/result
NX_DATE="$(date +%Y%m%d)"

echo ">>> Package TarBall."
cd $NX_DIR_SCRIPT/apps/CinemaDiagnostics/
tar cvzf $NX_DIR_SCRIPT/NxCinemaDiagnostics_$NX_DATE.tar.gz result --exclude="result/*.ilk" --exclude="result/*.pdb" --exclude="result/*.ini"

echo ">>> Package Zip."
cd $NX_DIR_SCRIPT/apps/CinemaDiagnostics/result
zip -r $NX_DIR_SCRIPT/NxCinemaDiagnostics_$NX_DATE.zip * -x "*.ilk" -x "*.pdb" -x "*.ini"

cd $NX_DIR_WORKING
