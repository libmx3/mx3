#import "MX3Api.h"
#import <mx3/mx3.hpp>
#import "objc_adapter.h"
using mx3::ObjcAdapter;

#include <string>
using std::string;

@implementation MX3Api {
  mx3::Api * __api;
}

- (MX3Api *) initWithPath:(NSString *)path {
  if(!(self = [super init])) {
    return nil;
  }
  __api = new mx3::Api(ObjcAdapter::convert(path));
  return self;
}

- (void) dealloc {
    delete __api, __api = nullptr;
}

- (BOOL) hasUser {
  return __api->has_user();
}

- (NSString *) getUsername {
  auto username = __api->get_username();
  return ObjcAdapter::convert(username);
}

- (void) setUsername:(NSString *)username {
  __api->set_username(ObjcAdapter::convert(username));
}

@end
