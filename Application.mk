# this is the android build file
NDK_TOOLCHAIN_VERSION := 4.8
APP_PLATFORM          := android-19
APP_STL               := gnustl_static
APP_ABI               := armeabi-v7a armeabi x86 # no mips, since leveldb doesn't have a mips atomic pointer yet
APP_BUILD_SCRIPT      := GypAndroid.mk
APP_MODULES           := libmx3_jni
