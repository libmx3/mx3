#pragma once
#include "gen/MX3EventLoop.h"

@interface MX3EventLoopObjc : NSObject <MX3EventLoop>

- (void)post:(id <MX3AsyncTask>)task;

@end
