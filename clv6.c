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
#define BUFLEN 1024
#define BACKLOG 5

char *ip6_ntoa(struct in6_addr ip6){
	static char str [INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &ip6, str, INET6_ADDRSTRLEN);
	return str;
}

struct in6_addr ip6_aton(char *str){
	struct in6_addr ip6;
	inet_pton(AF_INET6, str, &ip6);
	return ip6;
}

void subchar(char *t, const char *s){
	int len;
	int max = strlen(s);
	printf("len: %d\n", max);
	for(len = 0; len < max-1; len++){
		*t++ = *s++;
	}
	*t = '\0';
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

int tcp_connect(const char *host, const char *service){
	int err;
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct addrinfo *ai;
	int sock;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;
	
	err = getaddrinfo(host, service, &hints, &res);
	if(err != 0){
		printf("getaddrinfo(): %s\n", gai_strerror(err));
		return -1;
	}
	
	ai = res;
	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if(sock < 0){
		return -1;
	}
	int cs;
	cs = connect(sock, ai->ai_addr, ai->ai_addrlen);
	if(cs < 0){
		freeaddrinfo(res);
		return -1;
		//perror("connect");
		//exit(1);
	}
	printf("connect.\n");
	/*
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
	*/
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

int client(const char *host, char *buf){
	int sock;
	sock = tcp_connect(host, PORT);
	if(sock < 0){
		close(sock);
		return -1;
		//perror("client");
		//exit(1);
	}
	
	char conbuf[] = "pull\n";
	write(sock, conbuf, strlen(conbuf));
	int read_size;
	char rebuf[BUFLEN];
	read_size = read_line(sock, rebuf);
	if(read_size != 0)
		printf("mes: %s\n", rebuf);
	subchar(buf, rebuf);
	printf("sub: %s\n", buf);
	printf("disconnect.\n");
	close(sock);
	return 0;
}

int main(){
	char buf[BUFLEN];
	struct in6_addr coaddr;
	char ip6[] = "2002::a00:27ff:fea9:d6a1";
	int con;
	con = client(ip6, buf);
	if(con < 0){
		printf("connection error!\n");
		return 0;
	}
	
	coaddr = ip6_aton(buf);
	int i;
	for(i=0; i<16; i++){
		printf("%02x", coaddr.s6_addr[i]);
		if(i % 2 == 1)
			printf(":");
	}
	
	printf("\n");
	return 0;
}
