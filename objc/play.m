#import <Foundation/Foundation.h>
#import "gen/MX3Api.h"
#import "gen/MX3ApiCppProxy.h"

int main() {
    id <MX3Api> api = [MX3ApiCppProxy createApi:@"../mx3"];
    if (![api hasUser]) {
        [api setUsername: NSUserName()];
    }
    NSString * username = [api getUsername];
    NSLog(@"Hello, %@", username);
    return 0;
}
