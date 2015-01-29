#include <Foundation/Foundation.h>
#include "MX3ThreadLauncherObjc.h"
#include "gen/MX3AsyncTask.h"

@implementation MX3ThreadLauncherObjc

- (void)startThread:(NSString *)name runFn:(id <MX3AsyncTask>)runFn {
    NSThread *thread = [[NSThread alloc] initWithTarget:runFn selector:@selector(execute) object:nil];
    if (name) {
        [thread setName:name];
    }
    [thread start];
}

@end
