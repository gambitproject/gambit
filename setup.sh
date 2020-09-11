#!/bin/bash

mkdir m4
aclocal
glibtoolize
automake --add-missing
autoconf
./configure --prefix="$(pwd)" --disable-gui
make -j
cd src/python
python setup.py build
python setup.py install