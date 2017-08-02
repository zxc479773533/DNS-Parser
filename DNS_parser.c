#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define DNS_A  0x01
#define DNS_CNAME 0x05
//#define DNS_AAAA 0x28

/* DNS header */
typedef struct {
    u_short id;
    u_short tag;
    u_short num_question;
    u_short num_answer;
    u_short num_authority;
    u_short num_appendix;
} dns_header;

/* about socket */
int socketfd;
struct sockaddr_in dest;

/* main function */
int DNSParser(void);

/* build DNS requset and send */
void send_dns_request(const char *dns_name);

/* receive and parse DNS response */
void parse_dns_response();

/* get the domain name from DNS request */
void parse_dns_name(u_char *buf, u_char *p, char *name, int *len);

int DNSParser(void) {

    /* get gateway informations */
    char addr[16];
    char *name;
    printf("Please enter your gateway informations:\n");
    scanf("%s", addr);
    printf("\n");

    /* create socket */
    socketfd = socket(AF_INET , SOCK_DGRAM , 0);
    if(socketfd < 0){
		perror("create socket failed\n");
		return 1;
	}

    /* fill the dest struct */
    bzero(&dest , sizeof(dest));
    dest.sin_family = AF_INET;
	dest.sin_port = htons(53);
	dest.sin_addr.s_addr = inet_addr(addr);

    /* main circle */
    while (1) {

        /* get domain name */
        char target[64];
        printf("Please enter a domain name: (q for exit.)\n");
        scanf("%s", target);
        if (target[0] == 'q' || target[0] == 'Q')
            break;
        else
            name = target;

        /* send DNS requset */
        send_dns_request(name);

        /* get DNS response */
        parse_dns_response();
    }

    return 0;
}

void send_dns_request(const char *dns_name) {

    /* definations */
    u_char buf[512];
    u_char question[128];
    int Q_len = 0;

    /* settle question */
    char *pos;
	u_char *p = question;
    int n;
    pos = (char*)dns_name;

    while (1) {

        /* get len */
        if (strstr(pos, ".") != NULL)
            n = strlen(pos) - strlen(strstr(pos, "."));
        else
            n = strlen(pos);
        
        /* copy */
        *p = (u_char)n;
        p += 1;
        memcpy(p, pos, n);
        p += n;
        Q_len += (n + 1);        

        /* end */
        if (strstr(pos, ".") == NULL) {
            *p = (u_char)0;
            p += 1;
            Q_len += 1;
            break;
        }
        pos += n + 1;
    }

    *((u_short *)p) = htons(1);
    p += 2;
    Q_len += 2;
    *((u_short *)p) = htons(1);
    Q_len += 2;    

    /* memcpy */
    dns_header DNS;
    DNS.id = htons(0xff00);
    DNS.tag = htons(0x0100);
    DNS.num_question = htons(1);
    DNS.num_answer = 0;
    DNS.num_authority = 0;
    DNS.num_appendix = 0;
    memcpy(buf, &DNS, 12);
    memcpy(buf + 12, &question, Q_len);
    sendto(socketfd, buf, Q_len + 12, 0, (struct sockaddr*)&dest, sizeof(struct sockaddr));
}

void parse_dns_response() {

    /* definations */
    u_char buf[1024];
    dns_header DNS;
    char cname[128] , aname[128] , ip[20];
    u_char netip[4];
    struct sockaddr_in addr;
    char *src_ip;
    u_int addr_len = sizeof(struct sockaddr_in);
    int count, len, type , ttl , data_len;

    /* receive DNS response */
    recvfrom(socketfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);

    /* parse head */
    memcpy(buf, &DNS, 12);
    u_char *p = buf + 12;

    /* move over questions */
    int flag;
    for (count = 0; count < ntohs(DNS.num_question); count++) {
        while (1) {
            flag = (int)p[0];
            p += (flag + 1);
            if (flag == 0)
                break;
        }
        p += 4;
    }

    /* parse answers */
    printf("Get Answer:\n");
    for (count = 0; count < ntohs(DNS.num_answer); count++) {
        bzero(aname , sizeof(aname));
        len = 0;
        parse_dns_name(buf , p , aname , &len);
        p += 2;
        type = htons(*((u_short*)p));
		p += 4;
		ttl = htonl(*((u_int*)p));
		p += 4;
		data_len = ntohs(*((u_short*)p));
		p += 2;

        if (type == DNS_A) {
            bzero(ip , sizeof(ip));
            if (data_len == 4) {
                memcpy(netip, p, data_len);
                inet_ntop(AF_INET, netip, ip, sizeof(struct sockaddr));
                printf("Domain name: %s\n", aname);
                printf("IP address: %s\n", ip);
                printf("Time to alive: %d\n\n", ttl);
            }
        }
        else if (type == DNS_CNAME) {
            bzero(cname , sizeof(cname));
            len = 0;
            parse_dns_name(buf, p, cname, &len);
            printf("Domain name: %s\n", aname);
            printf("Alias name: %s\n\n", cname);
        }
        p += data_len;
    }
}

void parse_dns_name(u_char *buf, u_char *p, char *name, int *len) {

    int flag;
    char *pos = name + *len;

    while (1) {

        /* end */
        flag = (int)p[0];
        if (flag == 0)
            break;

        /* judge pointers */
        if ((flag & 0xc0) == 0xc0) {
            p = buf + (int)p[1];
            parse_dns_name(buf, p, name, len);
            break;
        }
        /* copy */
        else {
            p += 1;
            memcpy(pos, p, flag);
            pos += flag;
            p += flag;
            *len += flag;
            if ((int)p[0] != 0) {
                memcpy(pos, ".", 1);
                pos += 1;
                *len += 1;
            }
        }
    }
}

int main(void) {
    DNSParser();
    return 0;
}