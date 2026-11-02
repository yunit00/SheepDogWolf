/* Http_Get with WinHTTP (http_get.h), part of Windows: the system's proxy settings and certificates. */
#include "http_get.h"

#include <windows.h>
#include <winhttp.h>
#include <stdlib.h>
#include <string.h>

#include <string>

namespace {

std::wstring Wide(const char *s)
{
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, 0, 0);
    std::wstring w(n > 0 ? n - 1 : 0, L'\0');
    if (n > 1)
        MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    return w;
}

/* closes a WinHTTP handle when it goes out of scope */
struct Handle {
    HINTERNET h;
    explicit Handle(HINTERNET handle) : h(handle) {}
    ~Handle()
    {
        if (h)
            WinHttpCloseHandle(h);
    }
};

} // namespace

char *Http_Get(const char *url, const char *const *headers, const char *userAgent, int timeoutMs, size_t *size)
{
    *size = 0;
    std::wstring wurl = Wide(url);
    URL_COMPONENTS parts = {};
    parts.dwStructSize = sizeof(parts);
    parts.dwHostNameLength = (DWORD)-1;
    parts.dwUrlPathLength = (DWORD)-1;
    parts.dwExtraInfoLength = (DWORD)-1;
    if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &parts) || parts.nScheme != INTERNET_SCHEME_HTTPS)
        return 0;
    std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
    std::wstring path(parts.lpszUrlPath, parts.dwUrlPathLength);
    if (parts.lpszExtraInfo)
        path.append(parts.lpszExtraInfo, parts.dwExtraInfoLength);

    Handle session(WinHttpOpen(Wide(userAgent).c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
                               WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session.h)
        return 0;
    WinHttpSetTimeouts(session.h, timeoutMs, timeoutMs, timeoutMs, timeoutMs);
    Handle connection(WinHttpConnect(session.h, host.c_str(), parts.nPort, 0));
    if (!connection.h)
        return 0;
    Handle request(WinHttpOpenRequest(connection.h, L"GET", path.c_str(), 0, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE));
    if (!request.h)
        return 0;
    std::wstring extra;
    for (const char *const *h = headers; h && *h; h++)
        extra += Wide(*h) + L"\r\n";
    if (!WinHttpSendRequest(request.h, extra.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : extra.c_str(),
                            extra.empty() ? 0 : (DWORD)-1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request.h, 0))
        return 0;
    DWORD status = 0, statusSize = sizeof(status);
    if (!WinHttpQueryHeaders(request.h, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX) ||
        status != 200)
        return 0;

    std::string body;
    for (;;) {
        DWORD available = 0, read = 0;
        if (!WinHttpQueryDataAvailable(request.h, &available))
            return 0;
        if (!available)
            break;
        size_t at = body.size();
        body.resize(at + available);
        if (!WinHttpReadData(request.h, &body[at], available, &read))
            return 0;
        body.resize(at + read);
    }
    char *out = (char *)malloc(body.size() + 1);
    if (!out)
        return 0;
    memcpy(out, body.data(), body.size());
    out[body.size()] = 0;
    *size = body.size();
    return out;
}
