#include "objc_adapter.h"

using mx3::ObjcAdapter;

NSString *
ObjcAdapter::convert(const std::string& s) {
  return [[NSString alloc] initWithCString:s.c_str() encoding:NSUTF8StringEncoding];
}

std::string
ObjcAdapter::convert(NSString * s) {
  return std::string{[s UTF8String]};
}
