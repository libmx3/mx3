#import "MX3Api.h"
#include "stl.hpp"
#import <mx3/mx3.hpp>
#import "objc_adapter.hpp"
#import "objc_event_loop.hpp"
using mx3::ObjcAdapter;
using mx3::objc::ObjcEventLoop;

@implementation MX3Api {
  std::unique_ptr<mx3::Api> __api;
}

- (instancetype) initWithRootPath:(NSString *)path {
  if(!(self = [super init])) {
    return nil;
  }
  __api = make_unique<mx3::Api>(
    ObjcAdapter::convert(path),
    make_shared<ObjcEventLoop>(dispatch_get_main_queue())
  );
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
