/*
 * file_utilities.c
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <libwebsockets.h>
#include "file_utilities.h"

unsigned char *getVideoList(size_t *size) {
	char *videoDir;
	DIR *dir;
	struct dirent *entry;
	char *videoList;
	char *pos;

	videoDir = getenv(VIDEO_DIR_ENV_VAR);
	dir = opendir(videoDir);
	if (dir == NULL) {
		return NULL;
	}

	*size = strlen(VIDEO_LIST_KW);
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') {
			*size += 1 + strlen(entry->d_name);
		}
	}
	rewinddir(dir);

	videoList = (char *) malloc(LWS_SEND_BUFFER_PRE_PADDING + (*size * sizeof(char *)) + LWS_SEND_BUFFER_POST_PADDING);
	if (videoList == NULL) {
		closedir(dir);
		return NULL;
	}

	pos = &(videoList[LWS_SEND_BUFFER_PRE_PADDING]);
	strncpy(pos, VIDEO_LIST_KW, strlen(VIDEO_LIST_KW));
	pos += strlen(VIDEO_LIST_KW);
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') {
			*pos = '\t';
			pos++;
			strncpy(pos, entry->d_name, strlen(entry->d_name));
			pos += strlen(entry->d_name);
		}
	}
	closedir(dir);
	return (unsigned char *) videoList;
}

unsigned char *getDataSegFile(char *videoName, char *segNr, char *subDir, enum SegType type, size_t *size) {
	char *videoDir;
	char *filePath;
	char *pos;
	struct stat statBuf;
	int res;
	FILE *f;
	size_t read;
	unsigned char *data;

	videoDir = getenv(VIDEO_DIR_ENV_VAR);
	filePath = (char *) malloc((strlen(videoDir) + strlen(videoName) + 1 + strlen(subDir) + 2 + strlen(segNr) + 1) * sizeof(char));
	if (filePath == NULL) {
		return NULL;
	}

	pos = filePath;
	strcpy(pos, videoDir);
	pos += strlen(videoDir);
	strcpy(pos, videoName);
	pos += strlen(videoName);
	*pos = '/';
	pos++;
	strcpy(pos, subDir);
	pos += strlen(subDir);

	switch (type) {
	case DATA:
		strcpy(pos, "/d");
		pos += 2;
		break;
	case CODING:
		strcpy(pos, "/c");
		pos += 2;
		break;
	default:
		free(filePath);
		return NULL;
		break;
	}

	strcpy(pos, segNr);
	pos += strlen(segNr);
	*pos = '\0';

	res = stat(filePath, &statBuf);
	if (res < 0) {
		free(filePath);
		return NULL;
	}
	*size = statBuf.st_size;
	f = fopen(filePath, "r");
	free(filePath);
	if (f == NULL) {
		return NULL;
	}

	data = (unsigned char *) malloc(LWS_SEND_BUFFER_PRE_PADDING + (*size * sizeof(unsigned char)) + LWS_SEND_BUFFER_POST_PADDING);
	if (data == NULL) {
		fclose(f);
		return NULL;
	}
	read = fread(&data[LWS_SEND_BUFFER_PRE_PADDING], sizeof(unsigned char), *size, f);
	fclose(f);
	if (read != *size) {
		free(data);
		return NULL;
	}
	return data;
}

unsigned char *getInfoFile(char *videoName, char *filename, size_t *size) {
	char *videoDir;
	char *filePath;
	char *pos;
	struct stat statBuf;
	int res;
	FILE *f;
	size_t read;
	unsigned char *data;

	videoDir = getenv(VIDEO_DIR_ENV_VAR);
	filePath = (char *) malloc((strlen(videoDir) + strlen(videoName) + 1 + strlen(filename) + 1) * sizeof(char));
	if (filePath == NULL) {
		return NULL;
	}

	pos = filePath;
	strcpy(pos, videoDir);
	pos += strlen(videoDir);
	strcpy(pos, videoName);
	pos += strlen(videoName);
	*pos = '/';
	pos++;
	strcpy(pos, filename);
	pos += strlen(filename);
	*pos = '\0';

	res = stat(filePath, &statBuf);
	if (res < 0) {
		free(filePath);
		return NULL;
	}
	*size = statBuf.st_size;
	f = fopen(filePath, "r");
	free(filePath);
	if (f == NULL) {
		return NULL;
	}

	data = (unsigned char *) malloc(LWS_SEND_BUFFER_PRE_PADDING + (*size * sizeof(unsigned char)) + LWS_SEND_BUFFER_POST_PADDING);
	if (data == NULL) {
		fclose(f);
		return NULL;
	}
	read = fread(&data[LWS_SEND_BUFFER_PRE_PADDING], sizeof(unsigned char), *size, f);
	fclose(f);
	if (read != *size) {
		free(data);
		return NULL;
	}
	return data;
}
