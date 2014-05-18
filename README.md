## libmx3
A sample project showing how to use some cross platform libs.

### requirements
* `python` (required for gyp)
* xcode & `xcodebuild`
* android ndk and `ndk-build` on your PATH to build for android

### Quick Start
Run `make play && ./play`.  You should see "Hello, World" printed to the console, if so, you seem to already have
all the requirements met for building.

### building
running any of the make commands will automatically download [gyp](https://code.google.com/p/gyp/) and create
xcodeproj files for each platform.

build files are generated in `build_ios/mx3.xcodeproj`, `build_mac/mx3.xcodeproj`, and `GypAndroid.mk` but be careful,
running make commands again **will overwrite any changes you make to these files**. If you need to change something,
you should be able to do so from `common.gypi` or `mx3.gyp` and _recompile_ your buildfiles.

Make targets:
* `clean` - clean all generated files
* `ios`, `android`, `mac` - builds a static library for each platform
* `make play` - to write some quick "playground" style code in c++ (see play.cpp)
