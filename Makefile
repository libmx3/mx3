all: mac ios android

clean:
	-rm -rf build/
	-rm -rf deps/build/
	-rm -rf build_mac/
	-rm -rf build_ios/
	-rm -rf obj/
	-rm -rf libs/
	-rm -rf djinni-output-temp/
	-rm GypAndroid.mk
	-rm *.target.mk
	-rm deps/*.target.mk
	-rm -rf test_ldb
	-rm test.sqlite
	-rm play

gyp: ./deps/gyp

./deps/gyp:
	git submodule update --init

./deps/djinni:
	git submodule update --init

djinni-output-temp/gen.stamp mx3.cidl:
	./run_djinni.sh

djinni:
	./deps/gradle/gradlew djinni

# instruct gyp to build using the "xcode" build generator, also specify the OS
# (so we can conditionally compile using that var later)
build_mac/mx3.xcodeproj: deps/gyp deps/json11 mx3.gyp djinni
	PYTHONPATH=deps/gyp/pylib deps/gyp/gyp mx3.gyp -DOS=mac --depth=. -f xcode --generator-output=./build_mac -Icommon.gypi

build_ios/mx3.xcodeproj: deps/gyp deps/json11 mx3.gyp djinni
	PYTHONPATH=deps/gyp/pylib deps/gyp/gyp mx3.gyp -DOS=ios --depth=. -f xcode --generator-output=./build_ios -Icommon.gypi

GypAndroid.mk: deps/gyp deps/json11 mx3.gyp djinni
	ANDROID_BUILD_TOP=dirname PYTHONPATH=deps/gyp/pylib $(which ndk-build) deps/gyp/gyp --depth=. -f android -DOS=android --root-target libmx3_android -Icommon.gypi mx3.gyp

xb-prettifier := $(shell command -v xcpretty >/dev/null 2>&1 && echo "xcpretty -c" || echo "cat")

# a simple place to test stuff out
play: build_mac/mx3.xcodeproj objc/play.m
	xcodebuild -project build_mac/mx3.xcodeproj -configuration Debug -target play_objc | ${xb-prettifier} && ./build/Debug/play_objc

mac: build_mac/mx3.xcodeproj
	xcodebuild -project build_mac/mx3.xcodeproj -configuration Release -target libmx3_objc | ${xb-prettifier}

ios: build_ios/mx3.xcodeproj
	xcodebuild -project build_ios/mx3.xcodeproj -configuration Release -target libmx3_objc | ${xb-prettifier}

# This file needs to be manually configured per system.
example_android/local.properties:
	@echo "Android SDK and NDK not properly configured, please create a example_android/local.properties file." && false

android: GypAndroid.mk example_android/local.properties
	cd example_android && ./gradlew app:assembleDebug && cd ..

test: build_mac/mx3.xcodeproj
	xcodebuild -project build_mac/mx3.xcodeproj -configuration Debug -target test | ${xb-prettifier} && ./build/Debug/test

cleanup_gyp: ./deps/gyp mx3.gyp common.gypi
	deps/gyp/tools/pretty_gyp.py deps/gtest.gyp > gtest_temp.gyp && mv gtest_temp.gyp deps/gtest.gyp
	deps/gyp/tools/pretty_gyp.py deps/json11.gyp > json11_temp.gyp && mv json11_temp.gyp deps/json11.gyp
	deps/gyp/tools/pretty_gyp.py deps/sqlite3.gyp > sqlite3_temp.gyp && mv sqlite3_temp.gyp deps/sqlite3.gyp
	deps/gyp/tools/pretty_gyp.py mx3.gyp > mx3_temp.gyp && mv mx3_temp.gyp mx3.gyp
	deps/gyp/tools/pretty_gyp.py common.gypi > common_temp.gypi && mv common_temp.gypi common.gypi

.PHONY: djinni
