#include <stdio.h>

#include <string.h> /* for strncpy */
#include <unistd.h> /* for close */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <ifaddrs.h>

#define IF_NUM "eth3"

char *ip6_ntoa(struct in6_addr ip6){
	static char str [INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &ip6, str, INET6_ADDRSTRLEN);
	return str;
}

void getifipv6addr(struct in6_addr *ip6, const char *device){
	struct ifaddrs *if_list = NULL;
	struct ifaddrs *ifa = NULL;
	void *tmp = NULL;

	getifaddrs(&if_list);
	for(ifa = if_list; ifa != NULL; ifa = ifa->ifa_next){
		if(strcmp(ifa->ifa_name, device) == 0){
			if(!ifa->ifa_addr){
				continue;
			}else{
				if(ifa->ifa_addr->sa_family == AF_INET6){
					*ip6 = ((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
					break;
				}
			}
		}
	}
	freeifaddrs(if_list);
}

int main(){
	struct in6_addr ip6;
	getifipv6addr(&ip6, IF_NUM);
	printf("%s IPv6 address is %s.\n", IF_NUM, ip6_ntoa(ip6));
	return 0;
}
