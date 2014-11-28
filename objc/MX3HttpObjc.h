#pragma once
#include "gen/MX3Http.h"

@interface MX3HttpObjc : NSObject <MX3Http>

- (void)get:(NSString *)url callback:(id <MX3HttpCallback>)callback;

@end
