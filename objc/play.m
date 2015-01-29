#import <Foundation/Foundation.h>
#import "MX3HttpObjc.h"
#import "MX3ThreadLauncherObjc.h"

#import "MX3EventLoopObjc.h"
#import "gen/MX3Api.h"
#import "gen/MX3ApiCppProxy.h"


int main() {
    id <MX3Http> httpImpl = [[MX3HttpObjc alloc] init];
    id <MX3EventLoop> uiThread= [[MX3EventLoopObjc alloc] init];
    id <MX3ThreadLauncher> launcher = [[MX3ThreadLauncherObjc alloc] init];
    id <MX3Api> api = [MX3ApiCppProxy createApi:@"../mx3" uiThread: uiThread httpImpl:httpImpl launcher:launcher];
    if (![api hasUser]) {
        [api setUsername: NSUserName()];
    }
    NSString * username = [api getUsername];
    NSLog(@"Hello, %@", username);
    return 0;
}
