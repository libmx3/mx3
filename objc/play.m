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
    NSString *filePath = @"../mx3";
    BOOL fileExists =[[NSFileManager defaultManager] fileExistsAtPath:filePath];
    if (fileExists) {
        id <MX3Api> api = [MX3ApiCppProxy createApi:filePath uiThread: uiThread httpImpl:httpImpl launcher:launcher];
        if (![api hasUser]) {
            [api setUsername: NSUserName()];
        }
        NSString * username = [api getUsername];
        NSLog(@"Hello, %@", username);
    } else {
        NSLog(@"Could not find path: `%@`", filePath);
    }
    return 0;
}
