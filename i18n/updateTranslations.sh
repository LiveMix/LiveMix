#!/bin/sh

echo "Updating translation (*.ts) files"

cd ../

UI=`find . | grep "\.ui$"`
CPP=`find . | grep "\.cpp$"`
H=`find . | grep "\.h$"`
FILES="$UI $CPP $H"

CMD="$QTDIR/bin/lupdate-qt4 ${FILES} -ts"

$CMD i18n/livemix_fr.ts

echo "Creating *.qm files"
cd i18n
$QTDIR/bin/lrelease-qt4 *.ts


echo "Stats"
./stats.py
