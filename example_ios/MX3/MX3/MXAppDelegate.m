#import "MXAppDelegate.h"
#import "MX3Api.h"
#import "MXSampleDataTableViewController.h"

@implementation MXAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    [self setupDatabase];

    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    MXSampleDataTableViewController *sampleViewController = [[MXSampleDataTableViewController alloc] initWithStyle:UITableViewStylePlain];
    UINavigationController *navController = [[UINavigationController alloc] initWithRootViewController:sampleViewController];

    self.window.rootViewController = navController;
    self.window.backgroundColor = [UIColor whiteColor];
    [self.window makeKeyAndVisible];
    return YES;
}

- (void)setupDatabase {

    NSString *documentsFolder = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString *file = [documentsFolder stringByAppendingPathComponent:@"test_ldb"];

    MX3Api *api = [[MX3Api alloc] initWithPath:file];

    if (![api hasUser]) {
        NSLog(@"No user found, creating one");
        [api setUsername:NSUserName()];
    }
}

@end
