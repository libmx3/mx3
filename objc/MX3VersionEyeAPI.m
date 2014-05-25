#import <Foundation/Foundation.h>
#import "MX3VersionEyeAPI.h"
#import "Defines.h"

@implementation MX3VersionEyeAPI

+ (void)downloadProducts {
    NSString *URLString = [NSString stringWithFormat:@"https://www.versioneye.com/api/v2/products/search/mailbox?api_key=%@", APIKEY];
    NSURL *URL = [NSURL URLWithString:URLString];
    NSURLRequest *request = [NSURLRequest requestWithURL:URL];

    [NSURLConnection sendAsynchronousRequest:request
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {
        if (error) {
            NSLog(@"%@", error);
            return;
        }

        NSDictionary *dictionary = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
        NSLog(@"%@", dictionary);
    }];
}

@end
