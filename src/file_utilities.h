/*
 * file_utilities.h
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */

#ifndef FILE_UTILITIES_H_
#define FILE_UTILITIES_H_

#define VIDEO_DIR_ENV_VAR "VIDEO_DIR"
#define SCRIPT_ENV_VAR "SCRIPT_HOME"
#define BENTO4_ENV_VAR "BENTO4_HOME"
#define CRS_ENV_VAR "CRS_HOME"
#define NR_ENV_VARS 3

#define FRAG_SCRIPT "fragcodeMp4.sh"
#define REMOVE_COMMAND "rm -R "

#define MAX_SEND_SIZE 1024 * 1024

#define MPD_NAME "stream.mpd"
#define AUDIO_INIT_NAME "ainit.mp4"
#define VIDEO_INIT_NAME "vinit.mp4"

#define AUDIO_DIR "aenc"
#define VIDEO_DIR "venc"
#define DATA_DIR_SIZE 4

#define FILENAME_REGEX "^[a-zA-Z_.0-9]\\{1,64\\}$"

#define SWITCH_SERVER_KW "switch-server"
#define ACK_KW "OK"
#define NACK_KW "NOK"

#define VIDEO_LIST_KW "video-list"
#define MPD_KW "mpd-file"

#define K_STR_LEN 4
#define M_STR_LEN 4

static volatile int force_exit = 0;

char kStr[K_STR_LEN];
char mStr[M_STR_LEN];

enum Track {
	AUDIO, VIDEO
};

enum SegType {
	DATA, CODING
};

struct toSend {
	unsigned char *data;
	size_t size;
	size_t sent;
	int writeMode;
};

pthread_mutex_t mux;

unsigned char *getVideoList(size_t *size);

unsigned char *getEncodedSeg(char *videoName, char *segNr, enum Track t, enum SegType type, size_t *size);

unsigned char *getInfoFile(char *videoName, char *filename, size_t *size);

FILE *prepUpload(char *filename);

int startFragmentation(char *filename);

int freeIncompleteUpload(char *filename);

int str2int(char *str, int *i);

void *__fragmentation_worker(void *filePath);

char *__toStreamPath(char *uploadPath);

char *__getUploadDirPath(char *filename);

char *__getUploadDirName(char *filename);

int __validateUploadFilename(char *filename);

#endif /* FILE_UTILITIES_H_ */
