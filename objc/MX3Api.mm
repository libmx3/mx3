#import "MX3Api.h"
#include "stl.hpp"
#import <mx3/mx3.hpp>
#import "objc_adapter.hpp"
#import "objc_event_loop.hpp"
#import "objc_http.hpp"
#import "MX3QueryResult.h"
using mx3::ObjcAdapter;
using mx3::objc::ObjcEventLoop;
using mx3::objc::ObjcHttp;

@implementation MX3Api {
  std::unique_ptr<mx3::Api> __api;
}

- (instancetype) initWithRootPath:(NSString *)path {
  if(!(self = [super init])) {
    return nil;
  }
  __api = make_unique<mx3::Api>(
    ObjcAdapter::convert(path),
    make_shared<ObjcEventLoop>(dispatch_get_main_queue()),
    make_shared<ObjcHttp>()
  );
  return self;
}

- (void) dealloc {
    __api = nullptr;
}

- (MX3Snapshot *) launches {
    return [[MX3Snapshot alloc] initWithSnapshot: __api->get_launches()];
}

- (MX3QueryResult *) githubUsers {
    return [[MX3QueryResult alloc] initWithResult: __api->get_github_users()];
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
