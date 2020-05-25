#ifndef FILE_H
#define FILE_H

#include <stdlib.h>

/**
 * Get the PID of the current period process.
 * Return : 
 *      > 0 if period is not running
 *      > The PID if period is running  
 *      > -1 if error
 */
int get_period_pid();


/**
 * Check if period is already running, otherwise create /tmp/period.pid and write the pid inside
 * Return : 
 *      > The pid 
 *      > -1 if an error has occured (or period is already running)
 */ 
int write_pid();


/**
 * Create the pipe only if it doesn't already exists
 * Return : 
 *      > 0 if success
 *      > 1 if already exists
 *      > -1 if errors
 */ 
int make_pipe();

/**
 * Create the directory only if it doesn't already exists
 * Return : 
 *      > 0 if success
 *      > 1 if already exists
 *      > -1 if errors
 */ 
int make_dir();

int period_redirection();

int command_redirection(char type,size_t cmdId);

#endif