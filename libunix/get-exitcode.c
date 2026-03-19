#include <sys/types.h>
#include <sys/wait.h>
#include "libunix.h"

static int child_status_code(int st) {
    if(WIFEXITED(st))
        return WEXITSTATUS(st);
    if(WIFSIGNALED(st))
        return 128 + WTERMSIG(st);
    return st;
}

// non-blocking check if <pid> exited cleanly.
// returns:
//   - 0 if not exited;
//   - 1 if exited cleanly (exitcode in <status>, 
//   - -1 if exited with a crash (status holds reason)
int child_clean_exit_noblk(int pid, int *status) {
    int st = 0;
    pid_t r = waitpid(pid, &st, WNOHANG);

    if(r == 0)
        return 0;
    if(r < 0) {
        if(errno == EINTR)
            return 0;
        sys_die(waitpid, "child_clean_exit_noblk");
    }

    if(status)
        *status = child_status_code(st);

    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 1 : -1;
}

/*
 * blocking check that child <pid> exited cleanly.
 * returns:
 *  - 1 if exited cleanly, exitcode in <status>
 *  - 0 if crashed, reason in <status> .
 */
int child_clean_exit(int pid, int *status) {
    int st = 0;

    while(1) {
        pid_t r = waitpid(pid, &st, 0);
        if(r < 0) {
            if(errno == EINTR)
                continue;
            sys_die(waitpid, "child_clean_exit");
        }
        break;
    }

    if(status)
        *status = child_status_code(st);

    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 1 : 0;
}