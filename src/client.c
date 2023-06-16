#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server_client.h"
#include "../include/client.h"


int logged_in;
int client_socket,c_head_socket, c_sock_index;
struct Client *list;
fd_set c_master_list, c_watch_list;


/* Reference: Based on Demo Code posted on Piazza (02/22/2023) */
void client(char *port) 
{
	int c_selret;
	client_init(atoi(port));
    
    /* Zero select FD sets */

	FD_ZERO(&c_watch_list);
	FD_ZERO(&c_master_list);
	FD_SET(STDIN, &c_master_list);
	c_head_socket = STDIN;
	
	/* Register the listening socket */
	/* Register STDIN */
	// FD_SET(STDIN, &c_master_list);
	// c_head_socket = client_socket;
	while(TRUE){
		
		memcpy(&c_watch_list, &c_master_list, sizeof(c_master_list));
		// cse4589_print_and_log("have sockets: ");
		// cse4589_print_and_log("%d ", client_socket);
		// cse4589_print_and_log("%d\n", STDIN);
		// for (int fd = 0; fd < 100; fd++) {
		// 	if (FD_ISSET(fd, &c_watch_list)) {
		// 		printf("Before selecting, File descriptor %d is in the watch list\n", fd);
   		//  	}
		// }
		/* select() system call. This will BLOCK */
		c_selret = select(c_head_socket + 1, &c_watch_list, NULL, NULL, NULL);
		if(c_selret < 0)
			perror("select failed.");
		
		/* Check if we have sockets/STDIN to process */
		if(c_selret > 0){
			/* Loop through socket descriptors to check which ones are ready */
			for(c_sock_index=0; c_sock_index<=c_head_socket; c_sock_index+=1){
				
				if(FD_ISSET(c_sock_index, &c_watch_list)){
					/* Check if new command on STDIN */
					if (c_sock_index == STDIN){
						//Process PA1 commands here ...
						client_cmd(port);
						
					}
					/* Read from buffer */
					else{
						char *buffer = (char *) malloc(sizeof(char)*BUFFER_SIZE*20);
						char *temp = (char *) malloc(sizeof(char)*BUFFER_SIZE);
						memset(buffer, '\0', BUFFER_SIZE*20);
						memset(temp, '\0', BUFFER_SIZE);
			
						while (recv(c_sock_index, temp, BUFFER_SIZE, 0)>=0){
							strcat(buffer, temp);		
							if (strstr(temp, "完") != NULL)
								break;
							memset(temp, '\0', BUFFER_SIZE);
						}
						free(temp);

						buffer[strlen(buffer)-3] = '\0';
					
						if (strstr(buffer, "解除屏蔽成功") != NULL){
							cse4589_print_and_log("[%s:SUCCESS]\n", "UNBLOCK");
							cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
						}else if (strstr(buffer, "解除屏蔽失败") != NULL){
							cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
							cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
						}else if (strstr(buffer, "屏蔽成功") != NULL){
							cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCK");
							cse4589_print_and_log("[%s:END]\n", "BLOCK");
						}else if (strstr(buffer, "屏蔽失败") != NULL){
							cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
							cse4589_print_and_log("[%s:END]\n", "BLOCK");
						}else if (strstr(buffer, "说") != NULL){			
							msg_print(buffer);
						}else{
							get_list(buffer);
						}			
						free(buffer);	
					}
				}
			}
		}
	}
}


void buffered_msg_print(char *buffered_msg){
				char *msg= strtok(buffered_msg, "和");
				int index = 0;
				int idx = 0;
				char *array [20];
			while (msg!= NULL){
				
				char *temp1 = (char*) malloc(sizeof(char)*strlen(msg)+1);
				memset(temp1, '\0', strlen(msg)+1);
				memcpy(temp1, msg, sizeof(char)*strlen(msg));
				array[index]= temp1;
				msg = strtok(NULL,"和");
				index ++;

			}

			while (idx < index){
				char *message = array[idx];
				msg_print(array[idx]);
				free(array[idx]);
				idx++;
			}

}


void msg_print(char *buffer){
	char *client_ip;
	char *msg;
	client_ip = strtok(buffer, "说");
	msg = strtok(NULL, "说");
	cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
	cse4589_print_and_log("msg from:%s\n[msg]:%s\n", client_ip, msg);
	cse4589_print_and_log("[%s:END]\n", "RECEIVED");

}


//changed
void client_cmd(char *port)
{	
		char *array1;
		char portt[10];
		char *command_str = (char*) malloc(sizeof(char)*BUFFER_SIZE);
		// for sending buffer
		char *temp = (char*) malloc(sizeof(char)*BUFFER_SIZE);
		// for checking 
		char *temp2 = (char*) malloc(sizeof(char)*BUFFER_SIZE);
		memset(command_str, '\0', BUFFER_SIZE);
		memset(temp, '\0', BUFFER_SIZE);
		memset(temp2, '\0', BUFFER_SIZE);
		
		if(read(STDIN, command_str, BUFFER_SIZE-1) <= 0) // Mind the newline character that will be written to msg
			exit(-1);
		memcpy(temp, command_str, sizeof(char)*strlen(command_str));
		memcpy(temp2, command_str, sizeof(char)*strlen(command_str));
		
		/*check if command start with "LOGIN"*/
		
		if (strstr(command_str, "LOGIN") != NULL){
			char *command= strtok(command_str, " ");
			int index=0;
			char *array[3];
			int num=0;
			
			while (command!= NULL){
				array[index]= command;
				
				command = strtok(NULL, " ");
				index++;
				
			}
			 /*check if command start with "LOGIN"*/
			 /*exception case*/
   			int error=0;
			
			if (index!=3){
				/*enough input?*/
				error=1;
			}else{
					// fixed
					array1= (char*)malloc(sizeof(char)*strlen(array[1]));
					memcpy(array1, array[1], sizeof(char)*strlen(array[1]));
					array1[strlen(array[1])]='\0';
					struct in_addr ip_address;
    				int correct = inet_pton(AF_INET, array1, &ip_address);
					if (correct==0){
						error=1;
					}
					/*begin to check port*/
					int b;
					for (b=0; b<strlen(array[2]);b++){
						if(isalpha(array[2][b])){
							error=1;
							break;
						}
					}
					
					sprintf(portt, "%d", atoi(array[2]));
					if (atoi(array[2])<0 || atoi(array[2])>65535){
						error=1;
					}
					
				}
			

			if (error==1){
				cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
				cse4589_print_and_log("[%s:END]\n", "LOGIN");
			}else{
				if (logged_in==0){
					client_socket = connect_to_host(array1, portt);
					FD_SET(client_socket, &c_master_list);
					if(client_socket > c_head_socket) c_head_socket = client_socket;  
					
					logged_in=1;

					char c_port[20] ="PORT ";
					// changed;
					strcat(c_port, port);
					send(client_socket, c_port, strlen(c_port), 0);

					char *buffer = (char *) malloc(sizeof(char)*BUFFER_SIZE*100);
					char *temp = (char *) malloc(sizeof(char)*BUFFER_SIZE);
					memset(buffer, '\0', BUFFER_SIZE*100);
					memset(temp, '\0', BUFFER_SIZE);
			
					while (recv(client_socket, temp, BUFFER_SIZE, 0)>=0){
						strcat(buffer, temp);		
						if (strstr(temp, "完") != NULL)
							break;
						memset(temp, '\0', BUFFER_SIZE);
					}
					free(temp);

					buffer[strlen(buffer)-3] = '\0';

					if (strstr(buffer, "你") != NULL){
						char *array[2];
						char *message= strtok(buffer, "你");
						array[0]= message;
						message = strtok(NULL,"你");
						array[1]= message;
						buffered_msg_print(array[0]);
						get_list(array[1]);
					}else{
						get_list(buffer);
					}
					free(buffer);

					cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
					cse4589_print_and_log("[%s:END]\n", "LOGIN");
				}else{
					cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
					cse4589_print_and_log("[%s:END]\n", "LOGIN");
				}		
			}
		}else{
			command_str[strlen(command_str)-1] = '\0';
			temp[strlen(temp)-1] = '\0';
			temp2[strlen(temp2)-1] = '\0';
			
			if (!strcmp(command_str, "AUTHOR")) 
			{
				cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
				print_author();
				cse4589_print_and_log("[%s:END]\n", "AUTHOR");
			}
			else if (!strcmp(command_str, "IP")) 
			{	
				cse4589_print_and_log("[%s:SUCCESS]\n", "IP");
				print_ip();
				cse4589_print_and_log("[%s:END]\n", "IP");
			}
			else if (!strcmp(command_str, "PORT")) 
			{
				cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
				print_port();
				cse4589_print_and_log("[%s:END]\n", "PORT");
			}
			else if (!strcmp(command_str, "LIST")) 
			{
				cse4589_print_and_log("[%s:SUCCESS]\n", "LIST");
				client_print_list();
				cse4589_print_and_log("[%s:END]\n", "LIST");
			}
			else if (!strcmp(command_str, "REFRESH") && logged_in == 1) 
			{	
				cse4589_print_and_log("[%s:SUCCESS]\n", "REFRESH");
				refresh();
				cse4589_print_and_log("[%s:END]\n", "REFRESH");
			}
			else if (!strcmp(command_str, "EXIT")) 
			{
				cse4589_print_and_log("[%s:SUCCESS]\n", "EXIT");
				send(client_socket, temp, strlen(temp), 0);
				EXIT();
				cse4589_print_and_log("[%s:END]\n", "EXIT");
				exit(0);
			}
			else if (!strcmp(command_str, "LOGOUT") && logged_in == 1) 
			{
				cse4589_print_and_log("[%s:SUCCESS]\n", "LOGOUT");
				// reminder: removed from LOGIN-IN list in the server
				logged_in = 0;
				send(client_socket, temp, strlen(temp), 0);
				close(client_socket);
				FD_CLR(client_socket, &c_master_list);
				cse4589_print_and_log("[%s:END]\n", "LOGOUT");	
				
			}
			else if (strstr(command_str, "SEND") != NULL) 
			{
				int mark=0;	
				mark = check_ip(temp2);
			
				
				if (mark ==1){
					// reminder: <msg> to all logged-in clients. <msg> can have a maximum length of 256 bytes and will consist of valid ASCII characters.
					send(client_socket, temp, strlen(temp), 0);
					cse4589_print_and_log("[%s:SUCCESS]\n", "SEND");
					cse4589_print_and_log("[%s:END]\n", "SEND");
				}else{
					cse4589_print_and_log("[%s:ERROR]\n", "SEND");
					cse4589_print_and_log("[%s:END]\n", "SEND");
				}
				
			}
			else if (strstr(command_str, "BROADCAST") != NULL && logged_in == 1) 
			{	
				cse4589_print_and_log("[%s:SUCCESS]\n", "BROADCAST");
				//reminder: The client that executes BROADCAST should not receive the same message back.
				send(client_socket, temp, strlen(temp), 0);
				cse4589_print_and_log("[%s:END]\n", "BROADCAST");
				
			}
			else if (strstr(command_str, "UNBLOCK") != NULL) 
			{	
				int mark1=0;
				mark1 = check_ip(temp2);
				
				if (mark1 ==1 ){
					send(client_socket, temp, strlen(temp), 0);
				}else{
					cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
					cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
				}
			}
			else if (strstr(command_str, "BLOCK") != NULL) 
			{	
				int mark1=0;
				mark1 = check_ip(temp2);
			
				if (mark1 ==1 ){
					send(client_socket, temp, strlen(temp), 0);
				}else{
					cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
					cse4589_print_and_log("[%s:END]\n", "BLOCK");
				}
			}
			else 
			{
				cse4589_print_and_log("[%s:ERROR]\n", command_str);
				cse4589_print_and_log("[%s:END]\n", command_str);
			}
		}
	
	}



void client_init(int port)
{
	get_ip();
	set_port(port);
	logged_in = 0;
	client_socket = 1;
	list = NULL;	
}


/* Reference: Based on Demo Code posted on Piazza (03/03/2023) */
int connect_to_host(char *server_ip, char* server_port)
{	
	int fdsocket;
	struct addrinfo hints, *res;

	/* Set up hints structure */	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	/* Fill up address structures */	
	if (getaddrinfo(server_ip, server_port, &hints, &res) != 0)
		perror("getaddrinfo failed");
	
	/* Socket */
	fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fdsocket < 0)
		perror("Failed to create socket");
	
	/* Connect */
	if(connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
		perror("Connect failed");
	
	freeaddrinfo(res);
	
	return fdsocket;
}


void client_print_list() 
{	
	struct Client *head = list;
    int list_id = 1;
    while (head != NULL) 
    {
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id++, head->host_name, head->ip_addr, head->port_num);
        head = head->next;
    }
}


void refresh() {
	if (logged_in == 0)
		return;

	char *msg = "LIST";

	send(client_socket, msg, strlen(msg), 0);
}


void get_list(char *buffer) 
{	
	
	//changed

	int count=0;
	for (int a=0; a< strlen(buffer); a++){
		if (buffer[a] == ' '){
		count++;
		}
		
	}
	char a1[2] = " ";
	char a2[2] = "~";
	char *p = buffer;
	char **p_array= malloc(sizeof(char *)* count);
	char *buffered1 = strtok(buffer, a1);
	int i = 0;

	while (buffered1 != NULL) {
		char *a = malloc(sizeof(char)*(strlen(buffered1)+1));
		memcpy(a, buffered1, sizeof(char)*strlen(buffered1));
		a[strlen(buffered1)]='\0';
		p_array[i++] = a;
		buffered1 = strtok(NULL, a1);
	}

	free_list();
	struct Client *prev = NULL;
	for (i = 0; i < count; i++) {
			
		int index1 = 0;
		char *buffered2 = strtok(p_array[i], a2);
		
		char *info[4];
		while (buffered2 != NULL) {
		
		info[index1] = malloc(sizeof(char)*(strlen(buffered2)+1));
		memcpy(info[index1], buffered2, sizeof(char)*strlen(buffered2));
		(info[index1])[strlen(buffered2)]='\0';
		buffered2 = strtok(NULL, a2);
		
		index1++;
		}
			
		struct Client *elem = (struct Client *)malloc(sizeof(struct Client));
		memcpy(elem->host_name, info[1], sizeof(char)*strlen(info[1]));
		(elem->host_name)[strlen(info[1])] = '\0';
		memcpy(elem->ip_addr, info[2], sizeof(char)*strlen(info[2]));
		(elem->ip_addr)[strlen(info[2])] = '\0';
		elem->port_num = atoi(info[3]);
		elem->socket = 0;
		elem->next = NULL;

		if (list == NULL)
			list = elem;

		if (prev == NULL) 
		{
			prev = elem;
		}
		else 
		{
			prev->next = elem;
			prev = elem;
		}
	}
	
	
}


void free_list()
{
	while (list != NULL) {
		struct Client *temp = list;
		list = list -> next;
		free(temp);
	}
}


void EXIT()
{	
	if (logged_in == 1) {
		logged_in = 0;
		close(client_socket);
		FD_CLR(client_socket, &c_master_list);
	}	
}


//0 = fail 1=pass
int check_ip(char *temp2){
			int result =1;
			char *command[3];


			char *command_cut= strtok(temp2, " ");
			command[0]= command_cut;
			command_cut = strtok(NULL, " ");
			command[1]= command_cut;
			command_cut = strtok(NULL, " ");
			command[2]= command_cut;
			// invalid ip_address
			
			char *ip= (char*)malloc(sizeof(char)*strlen(command[1]));
			char *duplicate_ip= (char*)malloc(sizeof(char)*strlen(command[1]));
			
			memset(ip, '\0', sizeof(char)*strlen(command[1]));
			memset(duplicate_ip, '\0', sizeof(char)*strlen(command[1]));
			
			strncpy(ip, command[1], sizeof(char)*strlen(command[1]));
			strncpy(duplicate_ip, command[1], sizeof(char)*strlen(command[1]));
		
			ip[strlen(ip)]='\0';
			duplicate_ip[strlen(duplicate_ip)]='\0';
		
			result = validity_ip(ip);
			
		
			// Valid IP address which does not exist in the local copy of the list of logged-in clients
			if (result ==1){
				struct Client *head = list;
				int mark =0;
				while (head != NULL) 
				{

					if (!strncmp(duplicate_ip, head->ip_addr, strlen(head->ip_addr))){

						mark =1;
						break;
					}
					head = head->next;
				}
				result=mark;
			} 
			free(ip);
			free(duplicate_ip);

			return result;
}