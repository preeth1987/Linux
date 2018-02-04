#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <assert.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#define __USE_GNU
#include <dlfcn.h>

typedef int (*orig_socket_type)(int domain, int type, int protocol);

int socket(int domain, int type, int protocol)
{
	int socket;
	const char *vrf_name;
	int ret;

	vrf_name = getenv("VRF_NAME");
	if (vrf_name == NULL)
		goto error;

	orig_socket_type orig_socket;
	orig_socket = (orig_socket_type)dlsym(RTLD_NEXT,"socket");

	socket = orig_socket(domain, type, protocol);
	if (socket < 0)
		return socket;
	
	/*
	 * Bind to VRF
	 */
	ret = setsockopt(socket, SOL_SOCKET, SO_BINDTODEVICE, vrf_name, strlen(vrf_name)+1);
	if (ret < 0) {
		perror("socket option set failed");
		assert(0);
	}

error:
	return socket;
}
