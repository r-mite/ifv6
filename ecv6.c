#include <stdio.h>

#include <string.h> /* for strncpy */
#include <unistd.h> /* for close */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

int
main()
{
 int fd;
 struct ifreq ifr;

 fd = socket(AF_INET, SOCK_DGRAM, 0);

 /* IPv4のIPアドレスを取得したい */
 ifr.ifr_addr.sa_family = AF_INET;

 /* eth0のIPアドレスを取得したい */
 strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

 ioctl(fd, SIOCGIFADDR, &ifr);

 close(fd);

 /* 結果を表示 */
 printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

 return 0;
}
