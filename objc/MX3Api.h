#import <Foundation/Foundation.h>

// a small proxy objc implementation for mx3::Api
@interface MX3Api : NSObject
// todo(someone who knows objc better)
// is this supposed to return id or MX3Api * ?
- (MX3Api *)initWithPath:(NSString *)path;
- (void) dealloc;
- (BOOL) hasUser;
- (NSString *) getUsername;
- (void) setUsername:(NSString *) username;
@end
