## sx127x

### Compile:

    $ mkdir build
    $ cd build
    $ cmake ../
    $ make
    $ make install

### Cross Compile:

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake ../
    $ make
    $ make install
