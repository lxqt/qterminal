#!/usr/bin/env bash

QMAKE=qmake
MAKE=make
QTERMWIDGET_PATH=../qtermwidget

function die(){
    echo $1 && exit 1;
}

# test if QtermWidget sources are present
if [[ ! -d $QTERMWIDGET_PATH ]]
then
    die 'Error: qtermwidget not found. Please download its sources first...'
fi

# test if Qt of appropriate version is installed
qmake_qt4=`$QMAKE -v  | grep  "4\.[0-9]\.[0-9]"`
if [[ ! $qmake_qt4 ]]
then
    die 'Error: You need Qt4 library to build this project...'
fi

#Apply patches to QTermWidget
echo 'Patching QTermWidget component...'
cd qtermwidget_patches
patches=`ls *.patch`
cp  ${patches} ../${QTERMWIDGET_PATH} || die "Error copying patches into  ${QTERMWIDGET_PATH}"
cd ${OLDPWD}

cd ${QTERMWIDGET_PATH}
for patch in $patches
do
    patch -p0 < $patch
done

rm -f $patches
echo 'Done...'


# Compiling
cd ${OLDPWD}
echo 'Compiling project...'
$QMAKE && $MAKE
echo 'Done... Type `make install` as root now to install qterminal...'