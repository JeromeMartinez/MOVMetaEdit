#! /bin/sh

# Because of the autotools bug
cd MOV_MetaEdit/Project/GNU/CLI
./autogen.sh
cd ../../../..

./CLI_Compile.sh --enable-arch-x86_64 --enable-arch-i386
