#import "MX3Api+iOS.h"

static MX3Api *_sharedAPI;

@implementation MX3Api (iOS)

+ (MX3Api *)sharedAPI {
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        NSString *documentsFolder = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
        NSString *file = [documentsFolder stringByAppendingPathComponent:@"test_ldb"];
        _sharedAPI = [[MX3Api alloc] initWithPath:file];

        if (![_sharedAPI hasUser]) {
            NSLog(@"No user found, creating one");
            [_sharedAPI setUsername:NSUserName()];
        }
    });

    return _sharedAPI;
}

@end
