#import "MXAppDelegate.h"
#import "MXSampleDataTableViewController.h"
#import "MX3HttpObjc.h"
#import "MX3EventLoopObjc.h"
#import "MX3ThreadLauncherObjc.h"

@implementation MXAppDelegate

+ (MX3Api *) makeApi {
    // give mx3 a root folder to work in
    // todo, make sure that the path exists before passing it to mx3 c++
    NSString *documentsFolder = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSString *file = [documentsFolder stringByAppendingPathComponent:@"mx3"];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:file]) {
        [fileManager createDirectoryAtPath:file withIntermediateDirectories:NO attributes:nil error:nil];
    }
    
    id <MX3Http> httpImpl = [[MX3HttpObjc alloc] init];
    id <MX3EventLoop> uiThread= [[MX3EventLoopObjc alloc] init];
    id <MX3ThreadLauncher> launcher = [[MX3ThreadLauncherObjc alloc] init];
    MX3Api *api = [MX3Api createApi:file uiThread: uiThread httpImpl: httpImpl launcher:launcher];
    
    if (![api hasUser]) {
        NSLog(@"No user found, creating one");
        [api setUsername: @"djinni demo"];
    }
    return api;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.api = [MXAppDelegate makeApi];

    MXSampleDataTableViewController *sampleViewController = [[MXSampleDataTableViewController alloc] initWithApi:self.api];
    UINavigationController *navController = [[UINavigationController alloc] initWithRootViewController:sampleViewController];

    self.window.rootViewController = navController;
    self.window.backgroundColor = [UIColor whiteColor];
    [self.window makeKeyAndVisible];
    return YES;
}

@end
