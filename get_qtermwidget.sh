#!/usr/bin/env bash

echo 'Checking QTermWidget sources out...'
cd ..
cvs -d:pserver:anonymous@qtermwidget.cvs.sourceforge.net:/cvsroot/qtermwidget login
cvs -z3 -d:pserver:anonymous@qtermwidget.cvs.sourceforge.net:/cvsroot/qtermwidget co -P qtermwidget
cd ${OLDPWD}