/* The launcher's update check (update_check.h). */
#include "update_check.h"

#include "http_get.h"

#include <stdlib.h>

namespace {

SDL_Thread *s_thread;
SDL_AtomicInt s_stopping; /* the launcher is closing: the thread neither logs nor pushes an event */
SDL_AtomicInt s_done;     /* the thread has finished */

const int TIMEOUT_MS = 5000;

struct Job {
    char repository[128];
    int major, minor, patch; /* the launcher's version */
    Uint32 eventType;
};

/* "vX.Y.Z" (nothing after it) into its numbers; 0 when it is not one */
int ParseVersion(const char *s, int *major, int *minor, int *patch)
{
    int used = 0;
    return s && SDL_sscanf(s, "v%d.%d.%d%n", major, minor, patch, &used) == 3 && s[used] == 0;
}

/* owner/name, of the characters GitHub allows: nothing in it can change the URL */
int ValidRepository(const char *s)
{
    int slashes = 0;
    if (!s || !*s)
        return 0;
    for (; *s; s++) {
        if (*s == '/')
            slashes++;
        else if (!SDL_isalnum((unsigned char)*s) && *s != '-' && *s != '_' && *s != '.')
            return 0;
    }
    return slashes == 1;
}

/* the value of "key": "..." in the JSON text (the first one; GitHub's tag names have no escapes), into out */
int JsonString(const char *json, const char *key, char *out, size_t outSize)
{
    char quoted[64];
    SDL_snprintf(quoted, sizeof(quoted), "\"%s\"", key);
    const char *p = SDL_strstr(json, quoted);
    if (!p)
        return 0;
    p += SDL_strlen(quoted);
    while (*p == ' ' || *p == ':' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    if (*p != '"')
        return 0;
    const char *end = SDL_strchr(++p, '"');
    if (!end || (size_t)(end - p) >= outSize)
        return 0;
    SDL_memcpy(out, p, end - p);
    out[end - p] = 0;
    return 1;
}

int SDLCALL CheckThread(void *data)
{
    Job *job = (Job *)data;
    char url[256];
    SDL_snprintf(url, sizeof(url), "https://api.github.com/repos/%s/releases/latest", job->repository);
    const char *headers[] = {"Accept: application/vnd.github+json", 0};
    size_t size = 0;
    char *json = Http_Get(url, headers, "Rewooled-launcher", TIMEOUT_MS, &size);
    char tag[32];
    int major, minor, patch;
    if (SDL_GetAtomicInt(&s_stopping)) {
        /* the launcher is closing (and SDL may be gone): no answer */
    } else if (json && JsonString(json, "tag_name", tag, sizeof(tag)) && ParseVersion(tag, &major, &minor, &patch)) {
        bool newer = major != job->major ? major > job->major
                     : minor != job->minor ? minor > job->minor
                                           : patch > job->patch;
        SDL_Log("SheepLauncher: update check: the latest release is %s%s", tag, newer ? ", newer" : "");
        if (newer) {
            UpdateCheckResult *result = (UpdateCheckResult *)SDL_calloc(1, sizeof(UpdateCheckResult));
            SDL_strlcpy(result->version, tag, sizeof(result->version));
            SDL_snprintf(result->url, sizeof(result->url), "https://github.com/%s/releases/tag/%s", job->repository,
                         tag);
            SDL_Event e;
            SDL_zero(e);
            e.type = job->eventType;
            e.user.data1 = result;
            if (!SDL_PushEvent(&e))
                SDL_free(result);
        }
    } else {
        SDL_Log("SheepLauncher: update check: no answer");
    }
    free(json);
    SDL_free(job);
    SDL_SetAtomicInt(&s_done, 1);
    return 0;
}

} // namespace

int UpdateCheck_Start(const char *repository, const char *currentVersion, Uint32 eventType)
{
    int major, minor, patch;
    if (s_thread || !ValidRepository(repository) || !ParseVersion(currentVersion, &major, &minor, &patch))
        return 0; /* already checking, or not a release build, or not built from a repository */
    Job *job = (Job *)SDL_calloc(1, sizeof(Job));
    SDL_strlcpy(job->repository, repository, sizeof(job->repository));
    job->major = major;
    job->minor = minor;
    job->patch = patch;
    job->eventType = eventType;
    SDL_SetAtomicInt(&s_stopping, 0);
    SDL_SetAtomicInt(&s_done, 0);
    s_thread = SDL_CreateThread(CheckThread, "update check", job);
    if (!s_thread) {
        SDL_free(job);
        return 0;
    }
    return 1;
}

void UpdateCheck_Stop()
{
    if (!s_thread)
        return;
    SDL_SetAtomicInt(&s_stopping, 1);
    /* a request still waiting for GitHub is not interrupted: given half a second, then left to end on its own (it has
     * TIMEOUT_MS at most, and touches nothing of SDL's once stopping) */
    for (int waited = 0; !SDL_GetAtomicInt(&s_done) && waited < 500; waited += 10)
        SDL_Delay(10);
    if (SDL_GetAtomicInt(&s_done))
        SDL_WaitThread(s_thread, 0);
    else
        SDL_DetachThread(s_thread);
    s_thread = 0;
}

void UpdateCheck_Free(void *result)
{
    SDL_free(result);
}
