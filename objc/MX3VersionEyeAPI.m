#import <UIKit/UIKit.h>
#import "MX3VersionEyeAPI.h"
#import "Defines.h"

@implementation MX3VersionEyeAPI

+ (void)downloadProducts {
    NSString *URLString = [NSString stringWithFormat:@"https://www.versioneye.com/api/v2/products/search/mailbox?api_key=%@", APIKEY];
    NSURL *URL = [NSURL URLWithString:URLString];
    NSURLRequest *request = [NSURLRequest requestWithURL:URL];

    NSURLSession *session = [NSURLSession sharedSession];
    NSURLSessionDataTask *task = [session dataTaskWithRequest:request
                                            completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                if (error) {
                    NSLog(@"%@", error);
                    return;
                }

                NSDictionary *dictionary = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
                NSLog(@"%@", dictionary);
    }];

    [task resume];
}

@end
