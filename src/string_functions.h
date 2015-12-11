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

bool startsWith(const char *pre, const char *str);

void removeSubstring(char *s,const char *toremove);

char *stringAppender(char *s1, char *s2);

char *catVideoStringFactory(char *fileName);

char *catAudioStringFactory(char *fileName);

char *setVideoDirectory(char*fileName);

char *setAudioDirectory(char*fileName);

char *setMPDDirectory(char*fileName);

char *setVideoSegFile(char*fileName);


#endif /* STRING_FUNCTIONS_H_ */