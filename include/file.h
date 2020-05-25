#ifndef FILE_H
#define FILE_H

#include <stdlib.h>

/**
 * Get the PID of the current period process.
 * Return : 
 *      > The PID if period is running  
 *      > 0 if period is not running
 *      > exit on syscall failure
 */
int get_period_pid();


/**
 * Check if period is already running, otherwise create /tmp/period.pid and write the pid inside
 * Return : 
 *      > The pid written
 *      > -1 if period is already running
 *      > exit on syscall failure
 */ 
int write_pid();


/**
 * Create the pipe only if it doesn't already exists
 * Return : 
 *      > 0 if success
 *      > 1 if already exists
 *      > exit on syscall failure
 */ 
int make_pipe();

/**
 * Create the directory only if it doesn't already exists
 * Return : 
 *      > 0 if success
 *      > 1 if already exists
 *      > exit on syscall failure
 */ 
int make_dir();

/**
 * Do the period redirection
 * Return : 
 *      > void on success
 *      > exit on syscall failure
 */ 
void period_redirection();

/**
 * Do the command's redirection
 * type : The fd to redirect (input:i/output:o/error:o)
 * cmId : The command's id to redirects
 * Return : 
 *      > 0 on success
 *      > -1 if bad type
 *      > exit on syscall failure
 */ 
int command_redirection(char type,size_t cmdId);

#endif