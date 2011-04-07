
project(libvorbis C)

include_directories(${libvorbis_SOURCE_DIR}/../libogg ${libvorbis_SOURCE_DIR})

add_library(vorbis STATIC 
            analysis.c bitrate.c block.c codebook.c envelope.c floor0.c
            floor1.c info.c lookup.c lpc.c lsp.c mapping0.c mdct.c psy.c
            registry.c res0.c sharedbook.c smallft.c synthesis.c
            vorbisenc.c vorbisfile.c window.c)

