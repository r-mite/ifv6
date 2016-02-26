#include <stdio.h>
#include <stdlib.h>

#include <string.h> /* for strncpy */
#include <unistd.h> /* for close */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <netdb.h>

#define IF_NUM "eth3"
#define PORT "5000"
#define BUFLEN 256
#define BACKLOG 5

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

int tcplisten(const char *service){
	int err;
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct addrinfo *ai;
	int sock;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	//hints.ai_socktype = SOCK_STREAM;
	hints.ai_socktype = SOCK_DGRAM;	
	hints.ai_flags = AI_PASSIVE;
	
	err = getaddrinfo(NULL, service, &hints, &res);
	if(err != 0){
		printf("getaddrinfo(): %s\n", gai_strerror(err));
		return -1;
	}
	
	ai = res;
	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if(sock < 0){
		return -1;
	}
	int on = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
		return -1;
	}else{
		printf("set SO_REUSEADDR\n");
	}
	if(bind(sock, ai->ai_addr, ai->ai_addrlen) < 0)
		return -1;
	if(listen(sock, BACKLOG) < 0)
		return -1;
	freeaddrinfo(res);
	return sock;
}

int read_line(int sock, char *p){
	int len = 0;
	while(1){
		int ret;
		ret = read(sock, p, 1);
		if(ret == -1){
			perror("read");
			exit(1);
		}else if(ret == 0){
			break;
		}
		if(*p == '\n'){
			p++;
			len++;
			break;
		}
		p++;
		len++;
	}
	*p = '\0';
	return len;
}

void server(char *ip6){
	int sock;
	sock = tcplisten(PORT);
	if(sock < 0){
		perror("server");
		exit(1);
	}
	
	printf("PORT: %s, wait...\n", PORT);
	while(1){
/*
		int cs;
		struct sockaddr_storage sa;
		socklen_t len = sizeof(sa);
		cs = accept(sock, (struct sockaddr *)&sa, &len);
		if(cs<0){
			perror("accept");
			exit(1);
		}
		printf("accept.\n");
*/		
		int read_size;
		char buf[BUFLEN];
		//read_size = read_line(cs, buf);
		//if(read_size == 0)break;
		struct sockaddr *src;
		socklen_t *len;
		recvfrom(sock, buf, sizeof(buf), 0, src, len);
		printf("mes: %s", buf);
		char writebuf[BUFLEN];
		sprintf(writebuf, "%s\n", ip6);
		//write(cs, writebuf, strlen(writebuf));
		sendto(sock, writebuf, sizeof(writebuf), 0, src, *len);
		
		printf("reject.\n");
		//close(cs);
	}
}

int main(){
	struct in6_addr ip6;
	getifipv6addr(&ip6, IF_NUM);
	printf("%s - IPv6 address is %s.\n", IF_NUM, ip6_ntoa(ip6));

	server(ip6_ntoa(ip6));
	return 0;
}
