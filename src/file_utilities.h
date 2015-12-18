/*
 * file_utilities.h
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */

#ifndef FILE_UTILITIES_H_
#define FILE_UTILITIES_H_

#define VIDEO_DIR_ENV_VAR "VIDEO_DIR"
#define BENTO4_ENV_VAR "BENTO4_HOME"
#define CRS_ENV_VAR "CRS_HOME"
#define NR_ENV_VARS 3

#define MAX_SEND_SIZE 1024 * 1024

#define MPD_NAME "stream.mpd"
#define AUDIO_INIT_NAME "ainit.mp4"
#define VIDEO_INIT_NAME "vinit.mp4"

#define AUDIO_DIR "aenc"
#define VIDEO_DIR "venc"
#define DATA_DIR_SIZE 4

#define SWITCH_SERVER_KW "switch-server"
#define NOK_KW "NOK"

#define VIDEO_LIST_KW "video-list"
#define MPD_KW "mpd-file"

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

unsigned char *getVideoList(size_t *size);

unsigned char *getInfoFile(char *videoName, char *filename, size_t *size);

unsigned char *getEncodedSeg(char *videoName, char *segNr, enum Track t, enum SegType type, size_t *size);

#endif /* FILE_UTILITIES_H_ */
