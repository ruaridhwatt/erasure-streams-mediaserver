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

char *catVideoStringFactory(char *fileName) {
	char *vidCat1 = "cat ";
	char *vidCat2 = "/video/1/*.m4f >> ";
	char *vidCat3 = "/video/segs.m4f";

	char *shortendFileN = malloc((sizeof(char) * strlen(fileName)) + 1);
	strcpy(shortendFileN,fileName);
	removeSubstring(shortendFileN, ".mp4");
	char *nameDir = stringAppender(shortendFileN, "dir");
	char *appendString1 = stringAppender(vidCat1, nameDir);
	char *appendString2 = stringAppender(appendString1, vidCat2);
	char *appendString3 = stringAppender(appendString2, nameDir);
	char *appendString4 = stringAppender(appendString3, vidCat3);

	free(nameDir);
	free(appendString1);
	free(appendString2);
	free(appendString3);
	free(shortendFileN);
	return appendString4;
}

char *catAudioStringFactory(char *fileName) {
	char *audCat1 = "cat ";
	char *audCat2 = "/audio/und/*.m4f >> ";
	char *audCat3 = "/audio/segs.m4f";

	char *shortendFileN = malloc((sizeof(char) * strlen(fileName)) + 1);
	strcpy(shortendFileN,fileName);
	removeSubstring(shortendFileN, ".mp4");
	char *nameDir = stringAppender(shortendFileN, "dir");
	char *appendString1 = stringAppender(audCat1, nameDir);
	char *appendString2 = stringAppender(appendString1, audCat2);
	char *appendString3 = stringAppender(appendString2, nameDir);
	char *appendString4 = stringAppender(appendString3, audCat3);

	free(nameDir);
	free(appendString1);
	free(appendString2);
	free(appendString3);
	free(shortendFileN);
	return appendString4;
}

char *setVideoDirectory(char*fileName) {
	char *shortendFileN = malloc(sizeof(char) * (strlen(fileName) + 1));
	strcpy(shortendFileN,fileName);
	removeSubstring(shortendFileN, ".mp4");
	char *directory = "dir/video/1/";
	char *filedir = stringAppender(shortendFileN, directory);
	free(shortendFileN);
	return filedir;
}

char *setAudioDirectory(char*fileName) {
	char *shortendFileN = malloc(sizeof(char) * (strlen(fileName) + 1));
	strcpy(shortendFileN,fileName);
	removeSubstring(shortendFileN, ".mp4");
	char *directory = "dir/audio/und/";
	char *filedir = stringAppender(shortendFileN, directory);
	free(shortendFileN);
	return filedir;
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

