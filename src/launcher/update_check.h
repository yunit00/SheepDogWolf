/* The launcher's update check: whether the GitHub repository the build came from has a newer release.
 *
 * It asks GitHub's API for the latest release (https://api.github.com/repos/<repository>/releases/latest) with the
 * system's own networking (http_get.h: WinHTTP, NSURLSession, or the system's libcurl on Linux), on a thread of its
 * own, and compares its tag with the launcher's version, both vX.Y.Z. A build that is not a release (SDW_VERSION "dev",
 * a pull request's or a commit's name), or one built without SDW_REPOSITORY, does not check. Offline, without
 * releases, or on a Linux without libcurl: no answer, and nothing is shown. */
#ifndef SDW_UPDATE_CHECK_H
#define SDW_UPDATE_CHECK_H

#include <SDL3/SDL.h>

/* starts the check; the answer comes as an event of type eventType, data1 an UpdateCheckResult (to free with
 * UpdateCheck_Free) when there is a newer release, else no event at all. 0 when this build does not check. */
int UpdateCheck_Start(const char *repository, const char *currentVersion, Uint32 eventType);

struct UpdateCheckResult {
    char version[32]; /* the newer release's tag, e.g. v1.0.8 */
    char url[256];    /* its page, where the downloads are */
};

void UpdateCheck_Free(void *result);

/* before SDL_Quit: waits half a second at most for a check still waiting for GitHub, then leaves it to end on its own
 * (it no longer touches SDL's events once stopped) */
void UpdateCheck_Stop();

#endif
