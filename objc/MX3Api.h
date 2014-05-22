#include <Foundation/Foundation.h>
#include "MX3Snapshot.h"

// a small proxy objc implementation for mx3::Api
@interface MX3Api : NSObject
- (instancetype)initWithRootPath:(NSString *)path;
- (void) dealloc;
- (BOOL) hasUser;
- (MX3Snapshot *) launches;
- (NSString *) username;
- (void) setUsername:(NSString *) username;
@end
