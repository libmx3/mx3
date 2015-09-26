#include <Foundation/Foundation.h>
#include "MX3HttpObjc.h"
#include "gen/MX3HttpCallback.h"

@implementation MX3HttpObjc

- (void) get:(NSString *)urlString callback:(MX3HttpCallback *)callback {
    NSURL *URL = [NSURL URLWithString:urlString];
    NSURLRequest *request = [NSURLRequest requestWithURL:URL];
    
    [[[NSURLSession sharedSession] dataTaskWithRequest:request completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        if (error) {
            [callback onNetworkError];
        } else {
            int16_t httpCode = [(NSHTTPURLResponse*) response statusCode];
            NSString * strData = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            [callback onSuccess:httpCode data: strData];
        }
    }] resume];
}

@end
