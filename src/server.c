#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server_client.h"
#include "../include/server.h"


int server_socket, head_socket, sock_index;
fd_set master_list, watch_list;
struct Client *clients;
int conn_client_fd;
int first_list;


/* Reference: Based on Demo Code posted on Piazza (03/02/2023) */
void server(char *port) 
{
    server_init(atoi(port));

	struct addrinfo hints, *res;
	int selret;

	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
    	hints.ai_family = AF_INET;
    	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE;

	/* Fill up address structures */
	if (getaddrinfo(NULL, port, &hints, &res) != 0)
		perror("getaddrinfo failed");
	
	/* Socket */
	server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(server_socket < 0)
		perror("Cannot create socket");
	
	/* Bind */
	if(bind(server_socket, res->ai_addr, res->ai_addrlen) < 0 )
		perror("Bind failed");

	freeaddrinfo(res);
	
	/* Listen */
	if(listen(server_socket, BACKLOG) < 0)
		perror("Unable to listen on port");
	
	/* ---------------------------------------------------------------------------- */
	
	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	
	/* Register the listening socket */
	FD_SET(server_socket, &master_list);
	/* Register STDIN */
	FD_SET(STDIN, &master_list);
	
	head_socket = server_socket;
	
	while(TRUE){
		memcpy(&watch_list, &master_list, sizeof(master_list));
		
		/* select() system call. This will BLOCK */
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		if(selret < 0)
			perror("select failed.");
		
		/* Check if we have sockets/STDIN to process */
		if(selret > 0){
			/* Loop through socket descriptors to check which ones are ready */
			for(sock_index=0; sock_index<=head_socket; sock_index+=1){
				
				if(FD_ISSET(sock_index, &watch_list)){
					
					/* Check if new command on STDIN */
					if (sock_index == STDIN){
						//Process PA1 commands here ...
						server_cmd();
					}
					/* Check if new client is requesting connection */
					else if(sock_index == server_socket){
						client_request_conn();
					}
					/* Read from existing clients */
					else{
						client_request();	
					}
				}
			}
		}
	}
}


void server_init(int port)
{
    get_ip();
    set_port(port);
    static struct Client head = {"", "", 0, 0, 0, 0, 0, NULL, NULL, NULL};
    clients = &head;
}


/* Reference: Based on Demo Code posted on Piazza (02/22/2023) */
void server_cmd() 
{
    char *command_str = (char*) malloc(sizeof(char)*CMD_SIZE);
	memset(command_str, '\0', CMD_SIZE);
	if(fgets(command_str, CMD_SIZE-1, stdin) == NULL)
		exit(-1);
    command_str[strlen(command_str)-1] = '\0';

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
		server_print_list();
		cse4589_print_and_log("[%s:END]\n", "LIST");
	} 
	else if (!strcmp(command_str, "STATISTICS")) 
	{
		cse4589_print_and_log("[%s:SUCCESS]\n", "STATISTICS");
		statistics_list();
		cse4589_print_and_log("[%s:END]\n", "STATISTICS");
	}
	else if (strstr(command_str, "BLOCKED") != NULL) 
	{
		char *action = strtok(command_str, " ");
		char *client_ip = strtok(NULL, " ");
		struct Client *client = find_client(client_ip);

		// if invaild ip entered or client with this ip doesn't exist, client will be NULL
		if (client != NULL) 
		{
			cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCKED");
			block_list(client);
		}
		else 
		{
			cse4589_print_and_log("[%s:ERROR]\n", "BLOCKED");
		}
		cse4589_print_and_log("[%s:END]\n", "BLOCKED");
	}         
    else 
    {
        cse4589_print_and_log("[%s:ERROR]\n", command_str);
		cse4589_print_and_log("[%s:END]\n", command_str);
    }

    free(command_str);
}


void server_print_list() 
{
    struct Client *head = clients->next;
    int list_id = 1;
    while (head != NULL) 
    {
		if (head->status == 1)
        	cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id++, head->host_name, head->ip_addr, head->port_num);
		head = head->next;
    }
}


void statistics_list() 
{
    struct Client *head = clients->next;
    int list_id = 1;
    while (head != NULL) 
    {
        cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", list_id++, head->host_name, head->num_msg_sent, head->num_msg_rcv, head->status == 1 ? "logged-in" : "logged-out");
        head = head->next;
    }
}


void client_request() 
{
	/* Initialize buffer to receieve response */
	char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);
						
	if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
		close(sock_index);
							
		/* Remove from watched list */
		FD_CLR(sock_index, &master_list);
	}
	else{
	    //Process incoming data from existing clients here ...				
		if (!strcmp(buffer, "LIST")){
        	client_request_list();
        }
		else if (strstr(buffer, "SEND") != NULL){		
			client_send_msg(buffer);
		} 
		else if (strstr(buffer, "BROADCAST") != NULL){		
			client_broadcast_msg(buffer);
		} 
		else if (strstr(buffer, "UNBLOCK") != NULL){		
			client_unblock(buffer);
		} 
		else if (strstr(buffer, "BLOCK") != NULL){		
			client_block(buffer);
		} 
		else if (!strcmp(buffer, "LOGOUT")){		
			client_logout();
		}
		else if (!strcmp(buffer, "EXIT")){		
			client_exit();
		}  
	}

	free(buffer);
}


/* Reference: Based on Demo Code posted on Piazza (03/02/2023) */
void client_request_conn()
{
	int fdaccept=0;
    struct sockaddr_in client_addr;
    int caddr_len = sizeof(client_addr);
    char c_ip_addr[INET_ADDRSTRLEN];
    char c_host[1024];
    char c_service[20];
	struct Client *conn_client;
	struct String_Node *block_list_head;

	fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
	if(fdaccept < 0)
		perror("Accept failed.");

    /* Add to watched socket list */
	FD_SET(fdaccept, &master_list);
	if(fdaccept > head_socket) head_socket = fdaccept;  

	inet_ntop(AF_INET, &(client_addr.sin_addr), c_ip_addr, INET_ADDRSTRLEN);
    getnameinfo((struct sockaddr *)&client_addr, caddr_len, c_host, sizeof c_host, c_service, sizeof c_service, 0);

    conn_client = (struct Client *)malloc(sizeof(struct Client));

    memcpy(conn_client->host_name, c_host, sizeof(char)*strlen(c_host));
	(conn_client->host_name)[strlen(c_host)] = '\0';

    memcpy(conn_client->ip_addr, c_ip_addr, sizeof(char)*strlen(c_ip_addr));
	(conn_client->ip_addr)[strlen(c_ip_addr)] = '\0';

	conn_client->socket = fdaccept; 

	conn_client->status = 1;

	conn_client->num_msg_sent = 0;
	conn_client->num_msg_rcv = 0;

	conn_client->msg_buffer = NULL;

	block_list_head = (struct String_Node *)malloc(sizeof(struct String_Node));
	memset(block_list_head->data, '\0', 100+BUFFER_SIZE);
	block_list_head->next = NULL;
	conn_client->block_list = block_list_head; 

	char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);						
	if(recv(fdaccept, buffer, BUFFER_SIZE, 0) > 0){ 
		char *port_num = strtok(buffer, " ");
		port_num = strtok(NULL, " ");
		conn_client->port_num = atoi(port_num);
	}
	free(buffer);

	conn_client_fd = fdaccept;
	
	client_login(conn_client);

	first_list = 1;
	client_request_list();
	first_list = 0;
}


void client_login(struct Client *conn_client)
{
    struct Client *prev = clients;
    struct Client *cur = clients->next;

	// check if conn_client used to logged in
	while (cur != NULL) 
    {
        if (!strcmp(cur->ip_addr, conn_client->ip_addr)) 
		{
			cur->socket = conn_client->socket;
 			cur->status = 1;
			free(conn_client);

			// send messages received while offline
			while (cur->msg_buffer != NULL) 
			{	
				send(cur->socket, cur->msg_buffer->sender, strlen(cur->msg_buffer->sender), 0);
				send(cur->socket, "说", strlen("说"), 0);
				send(cur->socket, cur->msg_buffer->data, strlen(cur->msg_buffer->data), 0);
				cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");	
				cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", cur->msg_buffer->sender, cur->msg_buffer->receiver, cur->msg_buffer->data);
				cse4589_print_and_log("[%s:END]\n", "RELAYED");
				struct String_Node *temp = cur->msg_buffer;
				cur->msg_buffer = cur->msg_buffer->next;
				free(temp);

				if (cur->msg_buffer != NULL) 
				{
					send(cur->socket, "和", strlen("和"), 0);
				}
				else
				{
					send(cur->socket, "你", strlen("你"), 0);
				}
			}

			return;
		} 
        cur = cur->next;
    }

	// if first time logged in
	cur = clients->next;
    while (cur != NULL) 
    {
        if (conn_client->port_num <= cur->port_num)
            break;
        prev = cur;
        cur = cur->next;
    }
    prev->next = conn_client;
    conn_client->next = cur;
}


/* Reference: Based on Demo Code posted on Piazza (03/02/2023) */
void client_request_list() 
{	
    char *list = (char*)malloc(sizeof(char)*BUFFER_SIZE);
	memset(list, '\0', BUFFER_SIZE);
    struct Client *head = clients->next;
	int list_id = 1;
	char temp[10];
	int socket = first_list == 1 ? conn_client_fd : sock_index;
    while (head != NULL) 
	{
    	sprintf(temp, "%d", list_id++);
  		strcat(list, temp);
		strcat(list, "~");
      	strcat(list, head->host_name);
      	strcat(list, "~");
      	strcat(list, head->ip_addr);
      	strcat(list, "~");
      	sprintf(temp, "%d", head->port_num);
      	strcat(list, temp);
		strcat(list, " ");
    	head = head->next;
	}
	strcat(list, "完");
	send(socket, list, strlen(list), 0);
	free(list);
}


struct Client *find_cur_client()
{
	struct Client *cur = clients->next;
	while (cur != NULL) 
    {
        if (cur->socket == sock_index) 
			break;
		cur = cur->next;     
    }
	return cur;
}


struct Client *find_client(char *ip)
{
	struct Client *cur = clients->next;
	while (cur != NULL) 
    {
        if (!strcmp(cur->ip_addr, ip))
			break;
		cur = cur->next;  
    }
	return cur;
}


void client_send_msg(char *buffer)
{	char *buffer_copy = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer_copy, '\0', sizeof(char)*BUFFER_SIZE);
	memcpy(buffer_copy, buffer, sizeof(char)*strlen(buffer));
	
	char *action = strtok(buffer, " ");
	char *receiver_ip = strtok(NULL, " ");
	char *msg = buffer_copy + strlen(action) + strlen(receiver_ip) + 2;
	struct Client *sender = find_cur_client();
	struct Client *receiver = find_client(receiver_ip);
	struct String_Node *blocked_elem = find_blocked_elem(receiver, sender->ip_addr);
	struct String_Node *msg_buffer;
	struct String_Node *buffer_ptr;

	(sender->num_msg_sent)++;
	
	// receiver doesn't exist or receiver blocked sender
	if (receiver == NULL || blocked_elem != NULL) return;
	
	if (receiver->status == 1)
	{	
		// receiver is online, send msg
		send(receiver->socket, sender->ip_addr, strlen(sender->ip_addr), 0);
		send(receiver->socket, "说", strlen("说"), 0);
		send(receiver->socket, msg, strlen(msg), 0);
		send(receiver->socket, "完", strlen("完"), 0);
		cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");	
		cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", sender->ip_addr, receiver_ip, msg);
		cse4589_print_and_log("[%s:END]\n", "RELAYED");
	}
	else
	{
		// receiver is offline, buffer msg
		msg_buffer = (struct String_Node *)malloc(sizeof(struct String_Node));
		memcpy(msg_buffer->sender, sender->ip_addr, sizeof(char)*(strlen(sender->ip_addr)+1));
		memcpy(msg_buffer->receiver, receiver_ip, sizeof(char)*(strlen(receiver_ip)+1));
		memcpy(msg_buffer->data, msg, sizeof(char)*(strlen(msg)+1));
		msg_buffer->next = NULL;

		if (receiver->msg_buffer == NULL) {
			receiver->msg_buffer = msg_buffer;
		}	
		else 
		{
			buffer_ptr = receiver->msg_buffer;
			while (buffer_ptr->next != NULL)
				buffer_ptr = buffer_ptr->next;
			buffer_ptr->next = msg_buffer;
		}
	}
	(receiver->num_msg_rcv)++;

	free(buffer_copy);
}


void client_broadcast_msg(char *buffer) 
{
	char *buffer_copy = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer_copy, '\0', sizeof(char)*BUFFER_SIZE);
	memcpy(buffer_copy, buffer, sizeof(char)*strlen(buffer));

	char *action = strtok(buffer, " ");
	char *msg = buffer_copy + strlen(action) + 1;
	struct Client *sender = find_cur_client();
	struct Client *receiver;
	struct String_Node *blocked_elem;
	struct String_Node *msg_buffer;
	struct String_Node *buffer_ptr;

	(sender->num_msg_sent)++;

	receiver = clients->next;
	while (receiver != NULL) 
    {
		// don't send if the receiver is the sender theirself and if the receiver blocked the sender
		blocked_elem = find_blocked_elem(receiver, sender->ip_addr);
		if (!strcmp(sender->ip_addr, receiver->ip_addr) || blocked_elem != NULL) 
		{
			receiver = receiver->next;
			continue;
		}

		if (receiver->status == 1)
		{
			// receiver is online, send msg
			send(receiver->socket, sender->ip_addr, strlen(sender->ip_addr), 0);
			send(receiver->socket, "说", strlen("说"), 0);
			send(receiver->socket, msg, strlen(msg), 0);
			send(receiver->socket, "完", strlen("完"), 0);
			cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");	
			cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", sender->ip_addr, "255.255.255.255", msg);
			cse4589_print_and_log("[%s:END]\n", "RELAYED");
		}
		else
		{
			// receiver is offline, buffer msg
			msg_buffer = (struct String_Node *)malloc(sizeof(struct String_Node));
			memcpy(msg_buffer->sender, sender->ip_addr, sizeof(char)*(strlen(sender->ip_addr)+1));
			memcpy(msg_buffer->receiver, "255.255.255.255", sizeof(char)*(strlen("255.255.255.255")+1));
			memcpy(msg_buffer->data, msg, sizeof(char)*(strlen(msg)+1));
			msg_buffer->next = NULL;

			if (receiver->msg_buffer == NULL) {
				receiver->msg_buffer = msg_buffer;
			}	
			else 
			{
				buffer_ptr = receiver->msg_buffer;
				while (buffer_ptr->next != NULL)
					buffer_ptr = buffer_ptr->next;
				buffer_ptr->next = msg_buffer;
			}
		}
		(receiver->num_msg_rcv)++;   
		receiver = receiver->next; 
    }

	free(buffer_copy);
}


void client_block(char *buffer)
{
	struct Client *cur_client = find_cur_client();
	char *action = strtok(buffer, " ");
	char *blocked_ip = strtok(NULL, " ");

	struct Client *blocked_client = find_client(blocked_ip);
	struct String_Node *blocked_elem = find_blocked_elem(cur_client, blocked_ip);

	// blocked client doesn't exist or not logged in or already blocked
	if (blocked_client == NULL || blocked_client->status == 0 || blocked_elem != NULL) 
	{
		send(sock_index, "屏蔽失败完", strlen("屏蔽失败完"), 0);
		return;
	}

	// block the client
	struct String_Node *blocked_node = (struct String_Node *)malloc(sizeof(struct String_Node));
	memcpy(blocked_node->data, blocked_ip, sizeof(char)*strlen(blocked_ip));
	(blocked_node->data)[strlen(blocked_ip)] = '\0';

	blocked_node->next = cur_client->block_list->next;
	cur_client->block_list->next = blocked_node;

	send(sock_index, "屏蔽成功完", strlen("屏蔽成功完"), 0);
}


void client_unblock(char *buffer)
{
	struct Client *cur_client = find_cur_client();
	char *action = strtok(buffer, " ");
	char *unblocked_ip = strtok(NULL, " ");
	struct String_Node *block_prev = cur_client->block_list;;
	struct String_Node *block_cur = cur_client->block_list->next;

	// unblock the client
	while (block_cur != NULL) 
	{
		if (!strcmp(block_cur->data, unblocked_ip))
		{
			struct String_Node *temp = block_cur;
			block_prev->next = block_cur->next;
			free(temp);
			send(sock_index, "解除屏蔽成功完", strlen("解除屏蔽成功完"), 0);
			return;
		}
		block_prev = block_cur;
		block_cur = block_cur->next;
	}

	// unblocking client that is not blocked
	send(sock_index, "解除屏蔽失败完", strlen("解除屏蔽失败完"), 0);
}


struct String_Node *find_blocked_elem(struct Client *client, char *blocked_ip)
{
	if (client == NULL)
		return NULL;

	struct String_Node *cur = client->block_list->next;
	while (cur != NULL) 
	{
		if (!strcmp(cur->data, blocked_ip))
		{
			break;
		}
		cur = cur->next;
	}
	return cur;
}


void block_list(struct Client *client)
{
	struct String_Node *blocked_elem;
	struct Client *blocked_client;
	int list_id = 1;

	blocked_elem = client->block_list->next;
	while (blocked_elem != NULL)
	{
		blocked_client = find_client(blocked_elem->data);
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id++, blocked_client->host_name, blocked_client->ip_addr, blocked_client->port_num);
		blocked_elem = blocked_elem->next;
	}
}


void client_logout() 
{
    struct Client *cur = clients->next;
    while (cur != NULL) 
    {
		if (cur->socket == sock_index)
		{
			cur->status = 0;
			break;
		}
    	cur = cur->next;
    }
}


void client_exit() 
{
    struct Client *prev = clients;
    struct Client *cur = clients->next;
    while (cur != NULL) 
    {
		if (cur->socket == sock_index)
		{
			while (cur->block_list != NULL)
			{
				struct String_Node *temp = cur->block_list;
				cur->block_list = cur->block_list->next;
				free(temp);
			}

			struct Client *temp = cur;
			prev->next = cur->next;
			free(temp);
			break;
		}
		prev = cur;
    	cur = cur->next;
    }
}




