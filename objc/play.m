#import <Foundation/Foundation.h>
#import "MX3Api.h"

int main() {
    MX3Api * api = [[MX3Api alloc] initWithRootPath:@"../mx3"];
    if (![api hasUser]) {
        [api setUsername: NSUserName()];
    }
    NSString * username = [api username];
    NSLog(@"Hello, %@", username);
    return 0;
}
