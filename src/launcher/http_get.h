/* One HTTPS GET with the system's own networking, for the launcher's update check (update_check.cpp): no curl program
 * and no TLS library of ours. Windows: WinHTTP (http_get_win.cpp). macOS: NSURLSession (http_get_mac.mm). Linux and
 * other POSIX systems: the system's libcurl, loaded when it is needed (http_get_posix.cpp), so the launcher does not
 * depend on it: without it there is no check. */
#ifndef SDW_HTTP_GET_H
#define SDW_HTTP_GET_H

#include <stddef.h>

/* the body of a 200 answer to url, NUL-terminated, in a malloc'd buffer (free it), *size its length without the NUL;
 * 0 on any error, another status or no answer within timeoutMs. headers: extra "Name: value" lines, 0-terminated. */
char *Http_Get(const char *url, const char *const *headers, const char *userAgent, int timeoutMs, size_t *size);

#endif
