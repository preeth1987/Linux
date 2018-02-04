#define  _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/capability.h>
#include <sys/prctl.h>

#define   NEED_CAPS 1
static const cap_value_t need_caps[NEED_CAPS] = { CAP_NET_RAW };

int main(void)
{
    uid_t  real = getuid();
    cap_t  caps;

    /* Elevate privileges */
    if (setresuid(0, 0, 0))
        return 1; /* Fatal error, probably not setuid root */

    /* Add need_caps to current capabilities. */
    caps = cap_get_proc();
    if (cap_set_flag(caps, CAP_PERMITTED,   NEED_CAPS, need_caps, CAP_SET) ||
        cap_set_flag(caps, CAP_EFFECTIVE,   NEED_CAPS, need_caps, CAP_SET) ||
        cap_set_flag(caps, CAP_INHERITABLE, NEED_CAPS, need_caps, CAP_SET))
        return 1; /* Fatal error */

    /* Update capabilities */
    if (cap_set_proc(caps))
        return 1; /* Fatal error */

    /* Retain capabilities over an identity change */
    if (prctl(PR_SET_KEEPCAPS, 1L))
        return 1; /* Fatal error */

    /* Return to original, real-user identity */ 
    if (setresuid(real, real, real))
        return 1; /* Fatal error */

    /* Because the identity changed, we need to
     * re-install the effective set. */
    if (cap_set_proc(caps))
        return 1; /* Fatal error */

    /* Capability set is no longer needed. */
    cap_free(caps);

    /* You now have the CAP_NET_RAW capability.
     * It will be retained over fork() and exec().
    */

    return 0;
}
