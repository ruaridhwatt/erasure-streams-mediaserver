/*
 * file_utilities.h
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */

#ifndef FILE_UTILITIES_H_
#define FILE_UTILITIES_H_

#include <libwebsockets.h>
#include "llist.h"

#define VIDEO_DIR_ENV_VAR "VIDEO_DIR"
#define SCRIPT_ENV_VAR "SCRIPT_HOME"
#define BENTO4_ENV_VAR "BENTO4_HOME"
#define CRS_ENV_VAR "CRS_HOME"
#define NR_ENV_VARS 3

#define FRAG_SCRIPT "fragcodeMp4.sh"
#define REMOVE_COMMAND "rm -R "

#define RX_BUFFER_SIZE 4096

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

int kVal;
char kStr[K_STR_LEN];
int mVal;
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

pthread_mutex_t fmux;
pthread_mutex_t lmux;
llist *toDist;

/**
 * Returns a padded (ready to write to libwebsocket) tab separated list of videos avalable to stream. This is the
 * names of the sub-directories of the VIDEO_DIR environment variable
 * @param size Where the size of the resulting padded string should be stored
 * @return The padded tab separated list of video names
 */
unsigned char *getVideoList(size_t *size);

/**
 * Reads the specified segment to memory.
 * @param videoName The name of the video
 * @param segNr The segment number
 * @param track The track type
 * @param type the segment type (data or coding)
 * @param size Where the size of the file should be stored
 * @return The segment data
 */
unsigned char *getEncodedSeg(char *videoName, char *segNr, enum Track t, enum SegType type, size_t *size);

/**
 * Reads the specified info file (mpd or init segment) to memory.
 * @param videoName The video name
 * @param filename The info file name
 * @param size Where the size of the file should be stored
 * @return The info file data
 */
unsigned char *getInfoFile(char *videoName, char *filename, size_t *size);

/**
 * Checks that an upload of the specified file may take place. Creates a hidden folder with an open file pointer to
 * where the upload should be stored.
 * @param filename The proposed video (file) name
 * @return A pointer to where the upload should be written or NULL if the upload is not allowed.
 */
FILE *prepUpload(char *filename);

/**
 * Starts a worker thread to fragment and encode the video.
 *
 * @param filename The video filename
 * @return 0 on success otherwise -1
 */
int startFragmentation(char *filename);

/**
 * Frees all resources associated with an incomplete upload
 * @param filename The upload filename
 * @return 0 on success, otherwise -1
 */
int freeIncompleteUpload(char *filename);

/**
 * Converts a null terminated array of characters to an integer.
 * @param str The array of characters to be read.
 * @param i A pointer specifying where the conversion should be stored.
 * @return True if the conversion was successful, otherwise false.
 *          A conversion is considered successful iff all the str characters
 *          were used in the conversion and the result was within the range an
 *          int can store.
 */
int str2int(char *str, int *i);

void *__fragmentation_worker(void *filePath);

char *__toStreamPath(char *uploadPath);

char *__getUploadDirPath(char *filename);

char *__getUploadDirName(char *filename);

int __validateUploadFilename(char *filename);

#endif /* FILE_UTILITIES_H_ */
