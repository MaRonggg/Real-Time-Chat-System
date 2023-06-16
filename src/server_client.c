#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server_client.h"


char *ip_address;
int port_numbers;


void print_author()
{
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "xwu46");
}


void get_ip()
{
	// Reference: Demo Code posted on Piazza (02/22/2023)
	int fdsocket;
	struct addrinfo hints, *res;

	/* Set up hints structure */	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	/* Fill up address structures */	
	if (getaddrinfo("8.8.8.8", "53", &hints, &res) != 0)
		perror("getaddrinfo failed");

	/* Socket */
	fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fdsocket < 0)
		perror("Failed to create socket");
	
	/* Connect */
	if(connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
		perror("Connect failed");

	// Reference: Based on https://beej-zhcn.netdpi.net/man/inetntop_-_inetpton (02/22/2023)
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);
	static char ip_addr[INET_ADDRSTRLEN];

	getsockname(fdsocket, (struct sockaddr *)&sa, &sa_len);
	inet_ntop(AF_INET, &(sa.sin_addr), ip_addr, INET_ADDRSTRLEN);

	ip_address = ip_addr;

	freeaddrinfo(res);
	close(fdsocket);
}


void print_ip() 
{
	cse4589_print_and_log("IP:%s\n", ip_address);  
}


void set_port(int port)
{
	port_numbers = port;
}


void print_port() 
{
	cse4589_print_and_log("PORT:%d\n", port_numbers);
}


// 0 = false 1 = true
int validity_ip(char *ip){
	int result = 1;
	struct in_addr aip_address;
	char *ip_ad;
	ip_ad= (char*)malloc(sizeof(char)*strlen(ip));
	memcpy(ip_ad, ip, sizeof(char)*strlen(ip));
	ip_ad[strlen(ip)-1]='\0';
    int correct = inet_pton(AF_INET, ip_ad, &aip_address);
	if (correct==0){
		result=0;
	}
		
	return result;
}
int validity_port(char *port){
	int result=1;
	int b;
	char client_port[10];
	for (b=0; b<strlen(port);b++){
		if(isalpha(port[b])){
			result=0;
			break;
		}
	}

	sprintf(client_port, "%d", atoi(port));
	if (atoi(port)<0 || atoi(port)>65535){
		result=0;
	}
	return result;
}
