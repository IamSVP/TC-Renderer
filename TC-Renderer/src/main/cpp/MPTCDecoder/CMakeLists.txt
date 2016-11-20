include_directories(../${ndk_helper_dir}
                    ${ANDROID_NDK}/sources/android/cpufeatures
                    ${ANDROID_NDK}/sources/android/native_app_glue
                    ../)

SET(HEADERS
    "arithmetic_codec.h"
    "decoder.h"
    "wavelet.h"
    )

SET(SOURCES
    "arithmetic_codec.cpp"
    "decoder.cpp"
    "wavelet.cpp"
    )

add_library(mptc_decoder ${HEADERS} ${SOURCES})
MESSAGE(STATUS "MPTC Decoder is added!")
