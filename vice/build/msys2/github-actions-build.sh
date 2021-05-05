#!/usr/bin/env bash
set -o errexit
set -o nounset
cd "$(dirname $0)"/../..

#
# Build and install xa65. When this stops working, check
# https://www.floodgap.com/retrotech/xa/dists/ for a newer version.
#

XA_VERSION=2.3.11

if [ ! -e /usr/local/bin/xa65.exe ]
then
    pushd /usr/local
    mkdir -p src
    cd src
    wget https://www.floodgap.com/retrotech/xa/dists/xa-${XA_VERSION}.tar.gz
    tar -xzf xa-${XA_VERSION}.tar.gz
    cd xa-${XA_VERSION}
    make mingw install
    cp /usr/local/bin/xa.exe /usr/local/bin/xa65.exe
    popd
fi

# NOTE: --enable-ethernet removed,
# --disable-catweasel, --disable-hardsid, --disable-parsid, --disable-ssi2001 added,\
# to try to workaround the reported Windows need for one-time admin permissions to launch.

ARGS="\
    --disable-catweasel \
    --disable-hardsid \
    --disable-parsid \
    --disable-ssi2001 \
\
    --disable-arch \
    --disable-pdf-docs \
    --with-jpeg \
    --with-png \
    --with-gif \
    --with-vorbis \
    --with-flac \
    --enable-lame \
    --enable-midi \
    --enable-cpuhistory \
    --enable-external-ffmpeg \
    "

case "$1" in
GTK3)
    ARGS="--enable-native-gtk3ui $ARGS"
    ;;

SDL2)
    ARGS="--enable-sdlui2 $ARGS"
    ;;

*)
    echo "Bad UI: $1"
    exit 1
    ;;
esac

./autogen.sh
./configure $ARGS || ( echo -e "\n**** CONFIGURE FAILED ****\n" ; cat config.log ; exit 1 )
make -j $(( $NUMBER_OF_PROCESSORS )) -s
make bindistzip
