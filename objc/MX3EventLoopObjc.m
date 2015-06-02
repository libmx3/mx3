#include <Foundation/Foundation.h>
#include "MX3EventLoopObjc.h"
#include "gen/MX3AsyncTask.h"

@implementation MX3EventLoopObjc

- (void)post:(MX3AsyncTask *)task {
    dispatch_async(dispatch_get_main_queue(), ^{
        [task execute];
    });
}

@end
