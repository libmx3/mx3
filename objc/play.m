#import <Foundation/Foundation.h>
#import "MX3Api.h"

int main() {
    MX3Api * api = [[MX3Api alloc] initWithPath:@"../test_ldb"];
    if (![api hasUser]) {
        [api setUsername: NSUserName()];
    }
    NSString * username = [api username];
    NSLog(@"Hello, %@", username);
    return 0;
}
