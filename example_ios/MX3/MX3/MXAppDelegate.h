#import <UIKit/UIKit.h>
#import "gen/MX3Api.h"

@interface MXAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) id <MX3Api> api;

@end
