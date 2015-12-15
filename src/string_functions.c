/*
 * string_functions.c
 *
 *  Created on: Dec 9, 2015
 *      Author: dv11ann
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "string_functions.h"

bool startsWith(const char *pre, const char *str) {
	return strncmp(pre, str, strlen(pre)) == 0;
}

void removeSubstring(char *s,const char *toremove) {

	while((s=strstr(s,toremove)))
		memmove(s,s+strlen(toremove),1+strlen(s+strlen(toremove)));
}

char *stringAppender(char *s1, char *s2) {
	char *s3 = (char *)malloc((strlen(s1) + strlen(s2) + 2)*sizeof(char));
	strcpy(s3, s1);
	strcat(s3, s2);
	return s3;
}

char *setVideoDirectory(char*fileName) {
	char *shortendFileN = malloc(sizeof(char) * (strlen(fileName) + 1));
	strcpy(shortendFileN,fileName);
	removeSubstring(shortendFileN, ".mp4");

	char *directory = "/venc/";
	char *filedir = stringAppender(shortendFileN, directory);
	free(shortendFileN);
	return filedir;
}

char *setAudioDirectory(char*fileName) {
	char *shortendFileN = malloc(sizeof(char) * (strlen(fileName) + 1));
	strcpy(shortendFileN,fileName);
	removeSubstring(shortendFileN, ".mp4");

	char *directory = "/aenc/";
	char *filedir = stringAppender(shortendFileN, directory);
	free(shortendFileN);
	return filedir;
}

char *noMp4(char *fileName) {
	char *shortendFileN = malloc(sizeof(char) * (strlen(fileName) + 1));
	strcpy(shortendFileN,fileName);
	removeSubstring(shortendFileN, ".mp4");
	return shortendFileN;
}

char *setMPDDirectory(char*fileName) {
	char *shortendFileN = malloc(sizeof(char) * (strlen(fileName) + 1));
	strcpy(shortendFileN,fileName);
	removeSubstring(shortendFileN, ".mp4");
	char *directory = "dir/";
	char *filedir = stringAppender(shortendFileN, directory);
	fprintf(stderr, "hej2\n");
	free(shortendFileN);
	return filedir;
}

char *setVideoSegFile(char*fileName) {
	char *shortendFileN = malloc((sizeof(char) * strlen(fileName)) + 2);
	removeSubstring(shortendFileN, ".m4f");
	char *directory = "dir/video/";
	char *filedir = stringAppender(shortendFileN, directory);
	free(shortendFileN);
	return filedir;
}

