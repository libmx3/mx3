#import <Foundation/Foundation.h>
#import "MX3Api.h"

int main() {
    MX3Api * api = [[MX3Api alloc] initWithPath:@"../test_ldb"];
    if (![api hasUser]) {
        [api setUsername:@"steven"];
    }
    NSString * username = [api getUsername];
    NSLog(@"Hello, %@", username);
    return 0;
}
