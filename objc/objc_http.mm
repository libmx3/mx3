#include "objc_http.hpp"
#include <Foundation/Foundation.h>

using mx3::objc::ObjcHttp;
using mx3::HttpResponse;

namespace {
    NSString * s_to_nsstring(const string& str) {
        return [[NSString alloc] initWithCString:str.c_str() encoding:NSUTF8StringEncoding];
    }
}

void
ObjcHttp::get(const string& url, function<void(HttpResponse)> done_fn) {
    NSString *URLString   = s_to_nsstring(url);
    NSURL *URL            = [NSURL URLWithString:URLString];
    NSURLRequest *request = [NSURLRequest requestWithURL:URL];

    [NSURLConnection sendAsynchronousRequest:request
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {
        HttpResponse http_response;
        if (error) {
            http_response.http_code = 0;
            http_response.error = true;
            http_response.data = "";
        } else {
            http_response.http_code = [(NSHTTPURLResponse*) response statusCode];
            http_response.error = false;
            const char * bytes = static_cast<const char *>([data bytes]);
            size_t length      = [data length];
            http_response.data = std::string(bytes, bytes + length);
        }
        done_fn(http_response);
    }];
}
