#include <Foundation/Foundation.h>
#include "MX3HttpObjc.h"

@implementation MX3HttpObjc

- (void) get:(NSString *)urlString callback:(id <MX3HttpCallback>)callback {
    NSURL *URL            = [NSURL URLWithString:urlString];
    NSURLRequest *request = [NSURLRequest requestWithURL:URL];

    [NSURLConnection sendAsynchronousRequest:request
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {
        if (error) {
            [callback onNetworkError: [error code]];
        } else {
            int16_t httpCode = [(NSHTTPURLResponse*) response statusCode];
            NSString * strData = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            [callback onSuccess:httpCode data: strData];
        }
    }];
}

@end
