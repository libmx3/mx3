//
//  MX3AppDelegate.m
//  mx3
//
//  Created by Steven Kabbes on 5/18/14.
//  Copyright (c) 2014 Steven Kabbes. All rights reserved.
//

#import "MX3AppDelegate.h"
#import <MX3Api.h>

@implementation MX3AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  MX3Api * api = [[MX3Api alloc] initWithPath:@"./test_ldb"];
  if (![api hasUser]) {
    NSLog(@"No user found, creating one");
    [api setUsername:NSUserName()];
  }
  NSString * username = [api username];
  NSLog(@"Hello, %@", username);
}

@end
