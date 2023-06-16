#ifndef CLIENT_H_
#define CLIENT_H_

void client(char *); 

void client_init(int);

void client_cmd(char *);

int connect_to_host(char *, char *);

void client_print_list(); 

void refresh();

void get_list();

void free_list();

void msg_print(char *);

void EXIT();

int check_ip(char *temp2);

int check_block(char *temp2);

void buffered_msg_print(char *buffered_msg);

#endif