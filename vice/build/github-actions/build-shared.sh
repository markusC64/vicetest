#!/usr/bin/env bash
#
# Shared functions and variables for the Github Actions


# Shared command line options
ARGS="
    --disable-arch \
    --disable-pdf-docs \
    --with-png \
    --with-gif \
    --with-vorbis \
    --with-flac \
    --enable-ethernet \
    --enable-lame \
    --enable-midi \
    --enable-cpuhistory \
    --enable-ffmpeg \
    "

# Set configure options, check UI
#
# params:   $1  UI identifier (GTK3, SDL2)
#
set_configure_options()
{
    case "$1" in
        GTK3)
            ARGS="--enable-gtk3ui $ARGS"
            ;;
        SDL2)
            ARGS="--enable-sdl2ui $ARGS"
            ;;
        *)
            echo "Bad UI: $1"
            exit 1
            ;;
    esac

    # add option checking
    ARGS="--enable-option-checking=fatal $ARGS"
}
