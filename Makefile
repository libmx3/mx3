all: mac ios android

clean:
	-rm -rf build/
	-rm -rf build_mac/
	-rm -rf build_ios/
	-rm -rf obj/
	-rm play
	-rm GypAndroid.mk
	-rm *.target.mk

gyp: ./deps/gyp

./deps/gyp:
	git clone --depth 1 https://chromium.googlesource.com/external/gyp.git ./deps/gyp

# instruct gyp to build using the "xcode" build generator, also specify the OS
# (so we can conditionally compile using that var later)
mac_proj: deps/gyp
	deps/gyp/gyp mx3.gyp -DOS=mac --depth=. -f xcode --generator-output=./build_mac -Icommon.gypi

ios_proj: deps/gyp
	deps/gyp/gyp mx3.gyp -DOS=ios --depth=. -f xcode --generator-output=./build_ios -Icommon.gypi

android_proj: deps/gyp mx3.gyp
	ANDROID_BUILD_TOP=dirname $(which ndk-build) \
	deps/gyp/gyp --depth=. -f android \
	-DOS=android \
	--root-target libmx3 \
	-Icommon.gypi \
	mx3.gyp

# a simple place to test stuff out
play: mac_proj
	xcodebuild -project build_mac/mx3.xcodeproj -configuration Debug -target play && cp ./build/Debug/play ./play

mac: mac_proj
	xcodebuild -project build_mac/mx3.xcodeproj -configuration Release -target libmx3

ios: ios_proj
	xcodebuild -project build_ios/mx3.xcodeproj -configuration Release -target libmx3

android: android_proj
	GYP_CONFIGURATION=Release NDK_PROJECT_PATH=. ndk-build NDK_APPLICATION_MK=Application.mk -j4
