#ifndef MESSAGE_H
#define MESSAGE_H

int send_sring(int fd, const char *str);

char *recv_string(int fd);

int send_argv(int fd, char *argv[]);

char **recv_argv(int fd);


#endif 