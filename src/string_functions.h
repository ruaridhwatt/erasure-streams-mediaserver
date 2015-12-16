/*
 * string_functions.h
 *
 *  Created on: Dec 9, 2015
 *      Author: dv11ann
 */

#ifndef STRING_FUNCTIONS_H_
#define STRING_FUNCTIONS_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <unistd.h>

bool startsWith(const char *pre, const char *str);

void removeSubstring(char *s,const char *toremove);

char *stringAppender(char *s1, char *s2);

char *setVideoDirectory(char*fileName);

char *setAudioDirectory(char*fileName);

char *noMp4(char *fileName);

char *setVideoSegFile(char*fileName);

bool isDirectory(char *fileName);

#endif /* STRING_FUNCTIONS_H_ */
