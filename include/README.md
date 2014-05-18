No source files in this directory, just header include path.

Want to enable users of this library the ability to include files like:
```cpp
#include <mx3/mx3.hpp>
```

These files are essentially cross platform symlinks (preprocessor symlinks) to the real header files,
which exist in src/. The idea is stolen from openssl :)
