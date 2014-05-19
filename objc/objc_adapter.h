#pragma once
#include <Foundation/Foundation.h>
#include <string>

namespace mx3 {

// a helper class for converting between native types objc & c++
class ObjcAdapter final {
  public:
    static NSString * convert(const std::string& s);
    static std::string convert(NSString * s);
};

}
