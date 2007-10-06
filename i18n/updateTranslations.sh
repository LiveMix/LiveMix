#!/bin/sh

echo "Updating translation (*.ts) files"

cd ../

UI=`find . | grep "\.ui$"`
CPP=`find . | grep "\.cpp$"`
H=`find . | grep "\.h$"`
FILES="$UI $CPP $H"

if test "x$QTDIR" = "x"; then
  CMD="lupdate-qt4 ${FILES} -ts"
else
  CMD="$QTDIR/bin/lupdate-qt4 ${FILES} -ts"
fi

$CMD i18n/livemix_fr.ts

echo "Creating *.qm files"
cd i18n
if test "x$QTDIR" = "x"; then
  lrelease-qt4 *.ts
else
  $QTDIR/bin/lrelease-qt4 *.ts
fi

echo "Stats"
./stats.py
