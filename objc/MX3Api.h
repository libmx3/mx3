#import <Foundation/Foundation.h>
#import "MX3Snapshot.h"
#import "MX3QueryResult.h"

// a small proxy objc implementation for mx3::Api
@interface MX3Api : NSObject

- (instancetype) initWithRootPath:(NSString *)path;
- (void) dealloc;
- (BOOL) hasUser;
- (MX3Snapshot *) launches;
- (MX3QueryResult *) githubUsers;
- (NSString *) username;
- (void) setUsername:(NSString *) username;

@end
