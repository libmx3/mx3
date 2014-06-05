#import <Foundation/Foundation.h>
#ifdef __cplusplus
#include <memory>
#include <mx3/mx3.hpp>
#endif

@interface MX3QueryResult : NSObject
#ifdef __cplusplus
- (instancetype)initWithResult:(mx3::QueryResultPtr<github::User>) result;
#endif
// todo(pietbraur) let's make this a delegate?
- (void) listenToChanges: (void (^)()) changeBlock;
- (NSString *) rowAtIndex:(NSUInteger)index;
- (NSUInteger) count;

@end
