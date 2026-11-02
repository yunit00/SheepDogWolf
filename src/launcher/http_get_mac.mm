/* Http_Get with NSURLSession (http_get.h), part of macOS: the system's proxy settings and certificates. Compiled with
 * ARC (CMakeLists.txt). */
#include "http_get.h"

#import <Foundation/Foundation.h>

#include <stdlib.h>
#include <string.h>

char *Http_Get(const char *url, const char *const *headers, const char *userAgent, int timeoutMs, size_t *size)
{
    *size = 0;
    @autoreleasepool {
        NSURL *address = [NSURL URLWithString:[NSString stringWithUTF8String:url]];
        if (!address)
            return 0;
        NSTimeInterval seconds = timeoutMs / 1000.0;
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:address
                                                               cachePolicy:NSURLRequestReloadIgnoringLocalCacheData
                                                           timeoutInterval:seconds];
        [request setValue:[NSString stringWithUTF8String:userAgent] forHTTPHeaderField:@"User-Agent"];
        for (const char *const *h = headers; h && *h; h++) {
            const char *colon = strchr(*h, ':');
            if (!colon)
                continue;
            NSString *name = [[NSString alloc] initWithBytes:*h length:colon - *h encoding:NSUTF8StringEncoding];
            NSString *value = [[NSString stringWithUTF8String:colon + 1]
                stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            [request setValue:value forHTTPHeaderField:name];
        }

        /* an ephemeral session (nothing cached or stored), asked synchronously: this runs on the check's thread */
        NSURLSessionConfiguration *config = [NSURLSessionConfiguration ephemeralSessionConfiguration];
        config.timeoutIntervalForRequest = seconds;
        config.timeoutIntervalForResource = seconds;
        NSURLSession *session = [NSURLSession sessionWithConfiguration:config];
        __block NSData *body = nil;
        __block NSInteger status = 0;
        dispatch_semaphore_t done = dispatch_semaphore_create(0);
        NSURLSessionDataTask *task =
            [session dataTaskWithRequest:request
                       completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                         if (!error && [response isKindOfClass:[NSHTTPURLResponse class]]) {
                             status = ((NSHTTPURLResponse *)response).statusCode;
                             body = data;
                         }
                         dispatch_semaphore_signal(done);
                       }];
        [task resume];
        long late = dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, (int64_t)(timeoutMs + 1000) *
                                                                                       (int64_t)NSEC_PER_MSEC));
        [session invalidateAndCancel];
        if (late || status != 200 || !body)
            return 0;
        char *out = (char *)malloc(body.length + 1);
        if (!out)
            return 0;
        memcpy(out, body.bytes, body.length);
        out[body.length] = 0;
        *size = body.length;
        return out;
    }
}
