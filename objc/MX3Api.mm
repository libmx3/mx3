#import "MX3Api.h"
#import <mx3/mx3.hpp>
#import "objc_adapter.h"
using mx3::ObjcAdapter;

#include <string>
#include <memory>
using std::unique_ptr;
using std::string;

@implementation MX3Api {
  std::unique_ptr<mx3::Api> __api;
}

- (instancetype) initWithRootPath:(NSString *)path {
  if(!(self = [super init])) {
    return nil;
  }
  __api = unique_ptr<mx3::Api>(new mx3::Api(ObjcAdapter::convert(path)));
  return self;
}

- (void) dealloc {
    __api = nullptr;
}

- (MX3Snapshot *) launches {
    return [[MX3Snapshot alloc] initWithSnapshot: __api->get_launches()];
}

- (BOOL) hasUser {
  return __api->has_user();
}

- (NSString *) username {
  auto username = __api->get_username();
  return ObjcAdapter::convert(username);
}

- (void) setUsername:(NSString *)username {
  __api->set_username(ObjcAdapter::convert(username));
}

@end
