/*
 * erasure_server.h
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */

#ifndef ERASURE_SERVER_H_
#define ERASURE_SERVER_H_

/**
 * Verifies that the environment variables are set.
 * @param envVars The array of environment variables
 * @param nrVars The number of environment variables
 * @return 0 on success, otherwise -1
 */
int verifyEnvironmentSettings(char **envVars, size_t nrVars);

/**
 * Prints the usage statement to stdout
 * @param prog The name of the program binary. (argv[0])
 */
void print_usage(char *prog);

#endif /* ERASURE_SERVER_H_ */
