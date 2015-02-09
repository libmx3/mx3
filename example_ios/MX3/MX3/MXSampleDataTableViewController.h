#import <UIKit/UIKit.h>
#import "gen/MX3Api.h"
#import "gen/MX3UserListVmObserver.h"

@interface MXSampleDataTableViewController : UITableViewController <MX3UserListVmObserver>
- (instancetype) initWithApi:(id <MX3Api>) api;
- (void)onUpdate:(NSMutableArray *)changes newData:(id <MX3UserListVm>)newData;
@end
