#import <Foundation/Foundation.h>

@interface MX3GithubAPI : NSObject

/*
* Gets all Github users
* https://api.github.com/users
*/
+ (void)getUsersWithSuccess:(void (^)(id JSON))success failure:(void (^)(NSError *error))failure;

@end