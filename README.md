## libmx3
Cross platform has been well studied on desktop, but this is an exploration in doing that on mobile and an open request
for comments and improvements.

### the name
"mobile times 3", Android, iOS, and Windows Phone. (todo, windows phone support)

### requirements
* `python` (required for gyp)
* xcode & `xcodebuild`
* android ndk and `ndk-build` on your PATH to build for android
* windows phone developer tools (eventually...)

### Installation

1. Run `git submodule init` and `git submodule update` to download [json11](https://github.com/dropbox/json11).
2. Run `make play`. You should see "Hello, #{your login name}" printed to the console, if so, you seem to already have
all the requirements met for building.

### Objective-C

Run `make play ios` or `mac play mac`

### Building

Running any of the make commands will automatically download [gyp](https://code.google.com/p/gyp/) and create
`.xcodeproj` files for each platform.

Build files are generated in `build_ios/mx3.xcodeproj`, `build_mac/mx3.xcodeproj`, and `GypAndroid.mk` but be careful,
running make commands again **will overwrite any changes you make to these files**. If you need to change something,
you should be able to do so from `common.gypi` or `mx3.gyp` and _recompile_ your buildfiles.

Make targets:
* `clean` - clean all generated files
* `make test` - run the c++ tests
* `ios`, `android`, `mac` - builds a static library for each platform
* `make play` - to write some quick "playground" style code in objc (see objc/play.m)

### Directory structure

* mx3.gyp - gyp meta-build file for per-target settings
* common.gypi - a gyp _include_ which defines project wide settings
* Application.mk - the android make file
* Makefile - helper for interacting with gyp, and using command line builds (no xcode!!)
* deps/ - third party dependencies
* example\_mac/ - a _external_ xcodeproj which uses libmx3 ("the client" in a client/server model)
* include/ - header include paths, no code here
* objc/ - objc bindings to mx3::Api. Very simple api translation (NSString * -> std::string, lowerCamel -> under_lower, etc.)
* src/ - the c++ library, there should be no objc/java code here ("the server" in a client/server model)
* test/ - c++ tests
