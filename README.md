## libmx3
[![Build Status](https://travis-ci.org/libmx3/mx3.svg?branch=develop)](https://travis-ci.org/libmx3/mx3)
Cross platform has been well studied on desktop, but this is an exploration in doing that on mobile and an open request
for comments and improvements.

### The name
"mobile times 3", Android, iOS, and Windows Phone. (todo, windows phone support)

### Learning
* [Mobile C++ Slack instance](https://mobilecpp.herokuapp.com/)!
* [UIKonf 2014](http://oleb.net/blog/2014/05/how-dropbox-uses-cplusplus-cross-platform-development/)
* CppCon 2014 - [A Deep Dive Into Two Cross-Platform Mobile Apps Written in C++](https://www.youtube.com/watch?v=5AZMEm3rZ2Y)
* CppCon 2014 - [Practical Cross-Platform Mobile C++ Development](https://www.youtube.com/watch?v=ZcBtF-JWJhM)
* CppCon 2014 - [Microsoft w/ C++ to Deliver Office Across Different Platforms, Part II](https://www.youtube.com/watch?v=MGMoRu5yrVc)
* [Google Inbox](http://gmailblog.blogspot.com/2014/11/going-under-hood-of-inbox.html)
* [JVM at Google](http://medianetwork.oracle.com/video/player/3731062156001)
* Know of a good resource? put up a PR to add it!

### Tools of the Trade
* [djinni](https://github.com/dropbox/djinni) (pronounced genie) a tool which generates interface bindings from Java -> C++ and ObjC -> C++
* [gyp](https://chromium.googlesource.com/external/gyp/) a meta-build system used to generate native project files across platforms
* [cmake](http://www.cmake.org/) (not used in this project) an alternative to gyp
* [gradle](https://gradle.org/) build automation
* [json11](https://github.com/dropbox/json11) a minimalist C++11 library for json serialization
* [optional](https://github.com/akrzemi1/Optional) a reference implementation of [std::experimental::optional](http://en.cppreference.com/w/cpp/experimental/optional)
* [juce](http://www.juce.com/) cross platform C++ toolkit (not used here)
* [js2objc](https://github.com/google/j2objc) (not used in this project)
* [xamarin](http://xamarin.com/) (not used in the project)
* [Visual Studio's cross platform mobile support](https://www.visualstudio.com/en-us/features/cplusplus-mdd-vs.aspx) and a [related video](http://channel9.msdn.com/Events/Visual-Studio/Connect-event-2014/311)
* Know of a tool? put up a PR to add it!

### Contributing
There are many ways to contribute to mx3:

1. new proposals for architecture and techniques
1. bugfixes
1. documentation of any kind
1. new target platforms
1. educating us about your use case for cross platform mobile

There are a few things laid out in [TODO](TODO.md), if you want to contribute but don't know how.  If you are building a _large_
feature, please file an issue first to ensure that it aligns with the goals of mx3 - if it does, please submit
a Pull Request **during development** to solicit feedback and enable incremental code review throughout the process.

### Requirements
* `python` (required for gyp)
* xcode & `xcodebuild`
* android ndk and `ndk-build` on your PATH to build for android
* windows phone developer tools (eventually...)
* Configure the paths to your [Android SDK](http://developer.android.com/sdk/installing/index.html)
  and [Android NDK](http://developer.android.com/tools/sdk/ndk/index.html) by creating a file
  using the template at `example_android/local.properties.example`.

### Installation
1. Run `git submodule update --init` to download all dependencies
1. Optionally `gem install xcpretty` to make the output of `xcodebuild` nice
1. Run `make play`. You should see "Hello, #{your login name}" printed to the console, if so, you seem to already have
all the requirements met for building on iOS.
1. Run `make android` to build the example android application.

### Objective-C
Run `make play ios` or `make play mac`

### Android
Be sure to create a `example_android/local.properties` file pointing to the android-ndk and android-sdk.

### Building

Running any of the make commands will automatically download [gyp](https://code.google.com/p/gyp/) and create
`.xcodeproj` files for each platform.

Build files are generated in `build_ios/mx3.xcodeproj`, `build_mac/mx3.xcodeproj`, and `GypAndroid.mk` but be careful,
running make commands again **will overwrite any changes you make to these files**. If you need to change something,
you should be able to do so from `common.gypi` or `mx3.gyp` and _recompile_ your buildfiles.

Make targets:
* `clean` - clean all generated files
* `make test` - run the c++ tests
* `ios`, `mac` - builds a static library for each platform
* `android` - build the example app APK
* `make play` - to write some quick "playground" style code in objc (see objc/play.m)

### Directory structure

```bash
├── Application.mk # the android make file
├── Makefile # helper for interacting with gyp, and using command line builds (no xcode!!)
├── android/ # Java bindings to mx3::Api
├── common.gypi # a gyp include which defines project wide settings
├── deps/ # third party dependencies
├── example_android/ # the Android example project
├── example_ios/ # an external xcodeproj which uses libmx3 ("the client" in a client/server model)
├── include/ # header include paths, no code here
├── mx3.gyp # gyp meta-build file for per-target settings
├── objc/ # objc bindings to mx3::Api. Very simple api translation (NSString * -> std::string, lowerCamel -> under_lower, etc.)
├── src/ # the c++ library, there should be no objc/java code here ("the server" in a client/server model)
└── test/ # c++ tests
```
