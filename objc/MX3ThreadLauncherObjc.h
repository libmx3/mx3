#pragma once
#include "gen/MX3ThreadLauncher.h"

@interface MX3ThreadLauncherObjc : NSObject <MX3ThreadLauncher>

- (void)startThread:(NSString *)name runFn:(id <MX3AsyncTask>)runFn;

@end
