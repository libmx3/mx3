#import "MX3Api+iOS.h"

static MX3Api *_sharedAPI;

@implementation MX3Api (iOS)

+ (MX3Api *)sharedAPI {
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        NSString *documentsFolder = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
        // give mx3 a root folder to work in
        // todo, make sure that the path exists before passing it to mx3 c++
        NSString *file = [documentsFolder stringByAppendingPathComponent:@"mx3"];
        _sharedAPI = [[MX3Api alloc] initWithRootPath:file];

        if (![_sharedAPI hasUser]) {
            NSLog(@"No user found, creating one");
            [_sharedAPI setUsername:NSUserName()];
        }
    });

    return _sharedAPI;
}

@end
