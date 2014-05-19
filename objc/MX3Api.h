#import <Foundation/Foundation.h>

// a small proxy objc implementation for mx3::Api
@interface MX3Api : NSObject
- (instancetype)initWithPath:(NSString *)path;
- (void) dealloc;
- (BOOL) hasUser;
- (NSString *) username;
- (void) setUsername:(NSString *) username;
@end
