add_library(ndk-helper STATIC 
            gestureDetector.cpp
            GLContext.cpp
            interpolator.cpp
            JNIHelper.cpp
            perfMonitor.cpp
            tapCamera.cpp
            vecmath.cpp)

target_link_libraries(ndk-helper GLESv3)
target_include_directories(ndk-helper PRIVATE
                           ${ANDROID_NDK}/sources/android/native_app_glue)

