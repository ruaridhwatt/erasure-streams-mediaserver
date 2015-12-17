/*
 * video.c
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "file_utilities.h"
#include "video.h"

const int nrVideoCommands = 2;
const char *videoCommandStr[] = { "ini", "get" };
enum VideoCommand {
	GET_INIT = 0, GET_DATA_SEG = 1, UNKNOWN = 2
};

enum VideoCommand getVideoCommand(char *in) {
	int i;
	for (i = 0; i < nrVideoCommands; i++) {
		if (strcmp(in, videoCommandStr[i]) == 0) {
			break;
		}
	}
	return (enum VideoCommand) i;
}

int callback_video(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len) {
	enum VideoCommand c;
	struct toSend *s;
	int res;
	char *videoName;
	char *segNr;

	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		res = 0;
		c = getVideoCommand(strtok(in, "\t"));
		s = (struct toSend *) user;

		switch (c) {
		case GET_INIT:
			fprintf(stderr, "sending video init\n");
			s->writeMode = LWS_WRITE_BINARY;
			videoName = strtok(NULL, "\t");
			s->data = getInfoFile(videoName, VIDEO_INIT_NAME, &s->size);
			if (s->data == NULL) {
				fprintf(stderr, "Could not get video init!\n");
			}
			break;
		case GET_DATA_SEG:
			s->writeMode = LWS_WRITE_BINARY;
			videoName = strtok(NULL, "\t");
			segNr = strtok(NULL, "\t");
			fprintf(stderr, "sending video seg %s\n", segNr);
			s->data = getDataSegFile(videoName, segNr, VIDEO_DIR, DATA, &s->size);
			if (s->data == NULL) {
				fprintf(stderr, "Could not get video seg!\n");
			}
			break;
		default:
			fprintf(stderr, "Unknown command received!\n");
			break;
		}

		if (s->data == NULL) {
			break;
		}

		if (s->size < MAX_SEND_SIZE) {
			res = libwebsocket_write(wsi, &s->data[LWS_SEND_BUFFER_PRE_PADDING], s->size, s->writeMode);
			fprintf(stderr, "send res: %d\n", res);
			free(s->data);
			s->data = NULL;
			s->size = 0;
		} else {
			s->sent = 0;
			libwebsocket_callback_on_writable(ctx, wsi);
		}
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		res = 0;
		s = (struct toSend *) user;
		if (s->data == NULL) {
			break;
		}
		if (s->size - s->sent > MAX_SEND_SIZE) {
			res = libwebsocket_write(wsi, &s->data[LWS_SEND_BUFFER_PRE_PADDING + s->sent], MAX_SEND_SIZE, s->writeMode | LWS_WRITE_NO_FIN);
			fprintf(stderr, "send res: %d\n", res);
			if (res < 0) {
				free(s->data);
				s->data = NULL;
				s->size = 0;
			} else {
				s->writeMode = LWS_WRITE_CONTINUATION;
				s->sent += MAX_SEND_SIZE;
				libwebsocket_callback_on_writable(ctx, wsi);
			}
		} else {
			res = libwebsocket_write(wsi, &s->data[LWS_SEND_BUFFER_PRE_PADDING + s->sent], s->size - s->sent, s->writeMode);
			fprintf(stderr, "send res: %d\n", res);
			free(s->data);
			s->data = NULL;
			s->size = 0;
		}
		break;
	default:
		res = 0;
		break;
	}
	return res;
}

