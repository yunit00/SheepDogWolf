/* Http_Get with the system's libcurl, loaded when it is needed (http_get.h): the launcher does not link it, so it starts
 * without it, and then there is no update check. libcurl's options are passed by their numbers, from curl/curl.h (they
 * are part of its stable ABI), so its headers are not needed to build either. */
#include "http_get.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#ifndef HTTP_GET_LIBCURL
#define HTTP_GET_LIBCURL "libcurl.so.4" /* every libcurl since 7.16 (2006) */
#endif

namespace {

typedef void CURL;
struct curl_slist;
typedef CURL *(*EasyInit)();
typedef int (*EasySetopt)(CURL *, int, ...);
typedef int (*EasyPerform)(CURL *);
typedef int (*EasyGetinfo)(CURL *, int, ...);
typedef void (*EasyCleanup)(CURL *);
typedef curl_slist *(*SlistAppend)(curl_slist *, const char *);
typedef void (*SlistFreeAll)(curl_slist *);

/* curl/curl.h: CURLOPTTYPE_LONG 0, _OBJECTPOINT 10000, _FUNCTIONPOINT 20000; CURLINFO_LONG 0x200000 */
enum {
    CURLOPT_WRITEDATA = 10001,
    CURLOPT_URL = 10002,
    CURLOPT_USERAGENT = 10018,
    CURLOPT_HTTPHEADER = 10023,
    CURLOPT_FAILONERROR = 45,
    CURLOPT_FOLLOWLOCATION = 52,
    CURLOPT_NOSIGNAL = 99,
    CURLOPT_TIMEOUT_MS = 155,
    CURLOPT_WRITEFUNCTION = 20011,
    CURLINFO_RESPONSE_CODE = 0x200002,
};

struct Buffer {
    char *data;
    size_t size;
};

size_t Write(char *bytes, size_t size, size_t count, void *user)
{
    Buffer *b = (Buffer *)user;
    size_t n = size * count;
    char *grown = (char *)realloc(b->data, b->size + n + 1);
    if (!grown)
        return 0; /* curl then gives up */
    memcpy(grown + b->size, bytes, n);
    b->data = grown;
    b->size += n;
    b->data[b->size] = 0;
    return n;
}

} // namespace

char *Http_Get(const char *url, const char *const *headers, const char *userAgent, int timeoutMs, size_t *size)
{
    *size = 0;
    void *lib = dlopen(HTTP_GET_LIBCURL, RTLD_NOW | RTLD_LOCAL);
    if (!lib)
        return 0;
    EasyInit init = (EasyInit)dlsym(lib, "curl_easy_init");
    EasySetopt setopt = (EasySetopt)dlsym(lib, "curl_easy_setopt");
    EasyPerform perform = (EasyPerform)dlsym(lib, "curl_easy_perform");
    EasyGetinfo getinfo = (EasyGetinfo)dlsym(lib, "curl_easy_getinfo");
    EasyCleanup cleanup = (EasyCleanup)dlsym(lib, "curl_easy_cleanup");
    SlistAppend append = (SlistAppend)dlsym(lib, "curl_slist_append");
    SlistFreeAll freeAll = (SlistFreeAll)dlsym(lib, "curl_slist_free_all");
    char *out = 0;
    CURL *curl;
    if (init && setopt && perform && getinfo && cleanup && append && freeAll && (curl = init())) {
        curl_slist *list = 0;
        for (const char *const *h = headers; h && *h; h++)
            list = append(list, *h);
        Buffer body = {0, 0};
        long status = 0;
        setopt(curl, CURLOPT_URL, url);
        setopt(curl, CURLOPT_USERAGENT, userAgent);
        setopt(curl, CURLOPT_HTTPHEADER, list);
        setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        setopt(curl, CURLOPT_FAILONERROR, 1L);
        setopt(curl, CURLOPT_NOSIGNAL, 1L); /* on a thread: no SIGALRM for the timeout */
        setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeoutMs);
        setopt(curl, CURLOPT_WRITEFUNCTION, Write);
        setopt(curl, CURLOPT_WRITEDATA, &body);
        if (perform(curl) == 0 && getinfo(curl, CURLINFO_RESPONSE_CODE, &status) == 0 && status == 200 && body.data) {
            out = body.data;
            *size = body.size;
        } else {
            free(body.data);
        }
        freeAll(list);
        cleanup(curl);
    }
    /* not dlclose'd: libcurl and its TLS library register handlers that would run from unloaded code at exit */
    return out;
}
