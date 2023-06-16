#ifndef SERVER_H_
#define SERVER_H_

struct String_Node {
    char sender[20];
    char receiver[20];
    char data[100+BUFFER_SIZE];
    struct String_Node *next;
};

void server(char *); 

void server_init(int);

void server_cmd(); 

void server_print_list();

void statistics_list(); 

void client_request(); 

void client_request_conn();

void client_login(); 

void client_request_list(); 

struct Client *find_cur_client();

struct Client *find_client(char *);

void client_send_msg(char *);

void client_broadcast_msg(char *);

void client_block(char *);

void client_unblock(char *);

struct String_Node *find_blocked_elem(struct Client *, char *);

void block_list(struct Client *); 

void client_logout();

void client_exit();

#endif