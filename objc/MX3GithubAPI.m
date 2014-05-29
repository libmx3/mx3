#import "MX3GithubAPI.h"

@implementation MX3GithubAPI

static NSString *baseURL = @"https://api.github.com";

+ (void)getUsersWithSuccess:(void (^)(id JSON))success failure:(void (^)(NSError *error))failure {
    NSString *URLString = [NSString stringWithFormat:@"%@/users", baseURL];
    NSURL *URL = [NSURL URLWithString:URLString];
    NSURLRequest *request = [NSURLRequest requestWithURL:URL];

    [NSURLConnection sendAsynchronousRequest:request
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {
        if (error) {
            failure(error);
            return;
        }

        NSDictionary *dictionary = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
        success(dictionary);
    }];
}

@end