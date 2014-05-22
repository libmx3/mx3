#import <Foundation/Foundation.h>
#ifdef __cplusplus
#include <memory>
#include <mx3/mx3.hpp>
#endif

@interface MX3Snapshot : NSObject
// adding this here allows you to include this file from non-objc++ files
// since it is using c++ types
#ifdef __cplusplus
- (instancetype)initWithSnapshot:(std::unique_ptr<mx3::SqlSnapshot>)snapshot;
#endif
- (NSString *) rowAtIndex:(NSUInteger)index;
- (NSUInteger) count;
- (void) dealloc;
@end
