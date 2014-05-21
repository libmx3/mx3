#import <Foundation/Foundation.h>

// a small proxy objc implementation for mx3::Api
@interface MX3Api : NSObject
- (instancetype)initWithRootPath:(NSString *)path;
- (void) dealloc;
- (BOOL) hasUser;
- (NSString *) username;
- (void) setUsername:(NSString *) username;
@end
