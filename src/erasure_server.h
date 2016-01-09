/*
 * erasure_server.h
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */

#ifndef ERASURE_SERVER_H_
#define ERASURE_SERVER_H_

int verifyEnvironmentSettings(char **envVars, size_t nrVars);

void sighandler(int sig);

void print_usage(char *prog);

#endif /* ERASURE_SERVER_H_ */
