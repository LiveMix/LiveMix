#!/bin/sh

echo "Updating translation (*.ts) files"

cd ../

UI=`find . | grep "\.ui"`
CPP=`find . | grep "\.cpp"`
H=`find . | grep "\.h"`
FILES="$UI $CPP $H"

CMD="$QTDIR/bin/lupdate ${FILES} -ts"

$CMD i18n/livemix.fr.ts

echo "Creating *.qm files"
cd i18n
$QTDIR/bin/lrelease *.ts


echo "Stats"
./stats.py
