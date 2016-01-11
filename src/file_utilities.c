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
#include <regex.h>
#include <libwebsockets.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>
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
	pthread_mutex_lock(&fmux);
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') {
			*size += 1 + strlen(entry->d_name);
		}
	}
	rewinddir(dir);

	videoList = (char *) malloc(LWS_SEND_BUFFER_PRE_PADDING + ((*size + 1) * sizeof(char *)) + LWS_SEND_BUFFER_POST_PADDING);
	if (videoList == NULL) {
		pthread_mutex_unlock(&fmux);
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
	*pos = '\0';
	pthread_mutex_unlock(&fmux);
	closedir(dir);
	return (unsigned char *) videoList;
}

unsigned char *getEncodedSeg(char *videoName, char *segNr, enum Track track, enum SegType type, size_t *size) {
	char *videoDir;
	char *filePath;
	char *pos;
	struct stat statBuf;
	int res;
	FILE *f;
	size_t read;
	unsigned char *data;

	videoDir = getenv(VIDEO_DIR_ENV_VAR);
	filePath = (char *) malloc((strlen(videoDir) + strlen(videoName) + 1 + DATA_DIR_SIZE + 2 + strlen(segNr) + 1) * sizeof(char));
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

	switch (track) {
	case AUDIO:
		strcpy(pos, AUDIO_DIR);
		pos += strlen(AUDIO_DIR);
		break;
	case VIDEO:
		strcpy(pos, VIDEO_DIR);
		pos += strlen(VIDEO_DIR);
		break;
	default:
		free(filePath);
		return NULL;
	}

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

FILE *prepUpload(char *filename) {
	char *uploadDir, *streamDir, *pos, *filePath;
	FILE *f;

	uploadDir = __getUploadDirPath(filename);
	fprintf(stderr, "UploadDir: %s\n", uploadDir);
	if (uploadDir == NULL) {
		return NULL;
	}

	streamDir = __toStreamPath(uploadDir);
	fprintf(stderr, "StreamDir: %s\n", streamDir);
	if (streamDir == NULL) {
		free(uploadDir);
		return NULL;
	}

	pthread_mutex_lock(&fmux);
	if (access(streamDir, F_OK) == 0 || access(uploadDir, F_OK) == 0) {
		/* video exists */
		fprintf(stderr, "Already exists\n");
		pthread_mutex_unlock(&fmux);
		free(uploadDir);
		free(streamDir);
		return NULL;
	}

	free(streamDir);

	if (mkdir(uploadDir, S_IRWXU | S_IRWXG) != 0) {
		pthread_mutex_unlock(&fmux);
		free(uploadDir);
		return NULL;
	}
	pthread_mutex_unlock(&fmux);
	filePath = (char *) malloc((strlen(uploadDir) + strlen(filename) + 1) * sizeof(char));
	if (filePath == NULL) {
		free(uploadDir);
		return NULL;
	}
	pos = filePath;
	strncpy(pos, uploadDir, strlen(uploadDir));
	pos += strlen(uploadDir);
	free(uploadDir);
	strcpy(pos, filename);
	f = fopen(filePath, "wb");
	free(filePath);
	return f;
}

int startFragmentation(char *filename) {
	int res;
	char *uploadDir, *filePath;
	pthread_t detatchedWorker;
	pthread_attr_t attr;

	uploadDir = __getUploadDirPath(filename);
	if (uploadDir == NULL) {
		return -1;
	}
	filePath = (char *) malloc((strlen(uploadDir) + strlen(filename) + 1) * sizeof(char));
	if (filePath == NULL) {
		free(uploadDir);
		return -1;
	}
	strcpy(filePath, uploadDir);
	free(uploadDir);
	strcat(filePath, filename);
	free(filename);

	res = pthread_attr_init(&attr);
	if (res != 0) {
		free(filePath);
		return -1;
	}
	res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (res != 0) {
		free(filePath);
		pthread_attr_destroy(&attr);
		return -1;
	}
	res = pthread_create(&detatchedWorker, &attr, __fragmentation_worker, (void *) filePath);
	pthread_attr_destroy(&attr);
	if (res != 0) {
		free(filePath);
		return -1;
	}
	return 0;
}

int freeIncompleteUpload(char *filename) {
	int res;
	char *uploadDir, *command;

	uploadDir = __getUploadDirPath(filename);
	if (uploadDir == NULL) {
		return -1;
	}

	command = (char *) malloc((strlen(REMOVE_COMMAND) + strlen(uploadDir)) * sizeof(char));
	if (command == NULL) {
		free(uploadDir);
		return -1;
	}
	strcpy(command, REMOVE_COMMAND);
	strcat(command, uploadDir);
	free(uploadDir);
	pthread_mutex_lock(&fmux);
	res = system(command);
	pthread_mutex_unlock(&fmux);
	free(command);
	return res;
}


int str2int(char *str, int *i) {
	long l;
	char *pEnd;
	if (str == NULL) {
		return -1;
	}
	errno = 0;
	l = strtol(str, &pEnd, 10);
	if (pEnd == str || *pEnd != '\0' || errno == ERANGE) {
		return -1;
	}
	if (l > INT_MAX || l < INT_MIN) {
		errno = ERANGE;
		return -1;
	}
	*i = (int) l;
	return 0;
}

void *__fragmentation_worker(void *in) {
	int commandLen, res;
	char *scriptDir, *command, *pos, *streamDir, *filePath;

	filePath = (char *) in;

	scriptDir = getenv(SCRIPT_ENV_VAR);

	commandLen = strlen(scriptDir);
	commandLen += (strlen(FRAG_SCRIPT) + 1);
	commandLen += (strlen(filePath) + 1);
	commandLen += (strlen(kStr) + 1);
	commandLen += (strlen(mStr) + 1);

	command = (char *) malloc(commandLen * sizeof(char));
	if (command == NULL) {
		return NULL;
	}
	strcpy(command, scriptDir);
	strcat(command, FRAG_SCRIPT);
	strcat(command, " ");
	strcat(command, filePath);
	strcat(command, " ");
	strcat(command, kStr);
	strcat(command, " ");
	strcat(command, mStr);

	res = system(command);

	pos = strrchr(filePath, '/');
	*pos = '\0';
	if (res == 0) {
		streamDir = __toStreamPath(filePath);
		if (streamDir == NULL) {
			res = -1;
		} else {
			pthread_mutex_lock(&fmux);
			fprintf(stderr, "%s -> %s\n", filePath, streamDir);
			res = rename(filePath, streamDir);
			pthread_mutex_unlock(&fmux);
		}
	}

	if (res != 0) {
		free(streamDir);
		strcpy(command, REMOVE_COMMAND);
		strcat(command, filePath);
		pthread_mutex_lock(&fmux);
		system(command);
		pthread_mutex_unlock(&fmux);
	} else {
		pthread_mutex_lock(&lmux);
		llist_insert(llist_first(toDist), toDist, streamDir);
		pthread_mutex_unlock(&lmux);
	}
	free(command);
	free(filePath);
	return NULL;
}

char *__toStreamPath(char *uploadPath) {
	int i;
	char *streamPath, *pos;

	streamPath = (char *) malloc(strlen(uploadPath) * sizeof(char));
	if (streamPath == NULL) {
		return NULL;
	}
	pos = strrchr(uploadPath, '.');
	if (pos == NULL) {
		return NULL;
	}
	i = pos - uploadPath;
	strncpy(streamPath, uploadPath, i);
	pos++;
	strcpy(&streamPath[i], pos);
	return streamPath;
}

char *__getUploadDirPath(char *filename) {
	int res;
	char *videoHome, *subdir, *filepath, *pos;

	res = __validateUploadFilename(filename);
	fprintf(stderr, "valid filename? %s\n", res == 0 ? "Yes" : "No");
	if (res != 0) {
		return NULL;
	}
	videoHome = getenv(VIDEO_DIR_ENV_VAR);
	if (videoHome == NULL) {
		return NULL;
	}
	subdir = __getUploadDirName(filename);
	if (subdir == NULL) {
		return NULL;
	}
	filepath = (char *) malloc((strlen(videoHome) + strlen(subdir) + 2) * sizeof(char));
	if (filepath == NULL) {
		free(subdir);
		return NULL;
	}
	pos = filepath;
	strncpy(pos, videoHome, strlen(videoHome));
	pos += strlen(videoHome);
	strncpy(pos, subdir, strlen(subdir));
	pos += strlen(subdir);
	free(subdir);
	*pos = '/';
	pos++;
	*pos = '\0';

	return filepath;
}

char *__getUploadDirName(char *filename) {
	int i;
	char *dir, *pos;

	pos = strrchr(filename, '.');
	if (pos == NULL) {
		return filename;
	}
	i = pos - filename;
	if (i == 0) {
		return NULL;
	}
	dir = (char *) malloc((1 + i + 1) * sizeof(char));
	dir[0] = '.';
	strncpy(&dir[1], filename, i);
	dir[i + 1] = '\0';
	return dir;
}

int __validateUploadFilename(char *filename) {
	int res;
	regex_t regex;

	res = regcomp(&regex, FILENAME_REGEX, 0);
	if (res != 0) {
		return -1;
	}
	res = regexec(&regex, filename, 0, NULL, 0);
	regfree(&regex);
	return res;
}
