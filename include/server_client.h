#ifndef SERVER_CLIENT_H_
#define SERVER_CLIENT_H_

struct Client {
    char host_name[100];
    char ip_addr[100];
    int port_num;
    int socket;
    int status;
    int num_msg_sent;
    int num_msg_rcv;
    struct String_Node *msg_buffer;
    struct String_Node *block_list;
    struct Client *next;
};

void print_author();

void get_ip();

void print_ip(); 

void set_port(int);

void print_port();

int validity_ip(char *ip);

int validity_port(char *port);

#endif

