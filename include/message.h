#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * Send a string to a file descriptor
 * fd : The file descriptor
 * str : The string to send
 * Return : 
 *      > 0 on success
 *      > -1 on errors
 *      > exit on syscall failure
 */
int send_string(int fd, const char *str);

/**
 * Receive a string from a pipe
 * fd : the file descriptor
 * Return : 
 *      > The received string on success
 *      > NULL on errors
 *      > exit on syscall failure
 */
char *recv_string(int fd);

/**
 * Send an array of string in a pipe
 * fd : The file descriptor
 * argv : The array of string to send
 * Return : 
 *      > 0 on success
 *      > -1 on errors
 *      > exit on syscall failure
 */
int send_argv(int fd, char *argv[]);


/**
 * Receive an array of string from a pipe
 * fd : The file descriptor
 * Return : 
 *      > The received array on success
 *      > NULL on errors
 *      > exit on syscall failure
 */
char **recv_argv(int fd);


#endif 