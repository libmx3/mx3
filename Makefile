all: mac ios android

clean:
	-rm -rf build/
	-rm -rf deps/build/
	-rm -rf build_mac/
	-rm -rf build_ios/
	-rm -rf obj/
	-rm -rf libs/
	-rm GypAndroid.mk
	-rm *.target.mk
	-rm deps/*.target.mk
	-rm -rf test_ldb
	-rm test.sqlite
	-rm play
	make cleanup_gyp
	NDK_PROJECT_PATH=./example_android/android_mx3 ndk-build clean

gyp: ./deps/gyp

./deps/gyp:
	git clone --depth 1 https://chromium.googlesource.com/external/gyp.git ./deps/gyp

cleanup_gyp: ./deps/gyp mx3.gyp common.gypi
	deps/gyp/tools/pretty_gyp.py mx3.gyp > mx3_temp.gyp && mv mx3_temp.gyp mx3.gyp
	deps/gyp/tools/pretty_gyp.py common.gypi > common_temp.gypi && mv common_temp.gypi common.gypi

./deps/json11:
	git submodule update --init

# instruct gyp to build using the "xcode" build generator, also specify the OS
# (so we can conditionally compile using that var later)
build_mac/mx3.xcodeproj: deps/gyp deps/json11 mx3.gyp
	deps/gyp/gyp mx3.gyp -DOS=mac --depth=. -f xcode --generator-output=./build_mac -Icommon.gypi

build_ios/mx3.xcodeproj: deps/gyp deps/json11 mx3.gyp
	deps/gyp/gyp mx3.gyp -DOS=ios --depth=. -f xcode --generator-output=./build_ios -Icommon.gypi

GypAndroid.mk: deps/gyp deps/json11 mx3.gyp
	ANDROID_BUILD_TOP=dirname $(which ndk-build) deps/gyp/gyp --depth=. -f android -DOS=android --root-target libmx3_jni -Icommon.gypi mx3.gyp

xb-prettifier := $(shell command -v xcpretty >/dev/null 2>&1 && echo "xcpretty -c" || echo "cat")

# a simple place to test stuff out
play: build_mac/mx3.xcodeproj objc/play.m
	xcodebuild -project build_mac/mx3.xcodeproj -configuration Debug -target play_objc | ${xb-prettifier} && ./build/Debug/play_objc

mac: build_mac/mx3.xcodeproj
	xcodebuild -project build_mac/mx3.xcodeproj -configuration Release -target libmx3_objc | ${xb-prettifier}

ios: build_ios/mx3.xcodeproj
	xcodebuild -project build_ios/mx3.xcodeproj -configuration Release -target libmx3_objc | ${xb-prettifier}

android: GypAndroid.mk
	#GYP_CONFIGURATION=Release NDK_PROJECT_PATH=. ndk-build NDK_APPLICATION_MK=Application.mk -j4
	NDK_PROJECT_PATH=./example_android/android_mx3 ndk-build #NDK_APPLICATION_MK=Application.mk

test: build_mac/mx3.xcodeproj
	xcodebuild -project build_mac/mx3.xcodeproj -configuration Debug -target test | ${xb-prettifier} && ./build/Debug/test
