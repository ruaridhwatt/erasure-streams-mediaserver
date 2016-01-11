/**
 * info.c
 *
 *  Created on: 16 Dec 2015
 *      Author: dv12rwt
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "file_utilities.h"
#include "info.h"

const int nrInfoCommands = 2;
const char *infoCommandStr[] = { "lst", "mpd" };
enum InfoCommand {
	LIST_VIDEOS = 0, GET_MPD = 1, UNKNOWN = 2
};

/**
 * Converts an info command string to the corresponding InfoCommand enum
 * @param in The command string
 * @return The corresponding InfoCommand enum
 */
enum InfoCommand getInfoCommand(char *in) {
	int i;
	for (i = 0; i < nrInfoCommands; i++) {
		if (strcmp(in, infoCommandStr[i]) == 0) {
			break;
		}
	}
	return (enum InfoCommand) i;
}

/**
 * Callback for the "info" websockets protocol
 * @param ctx The context
 * @param wsi The client connection
 * @param reason The reason for the callback
 * @param user The allocated per user data
 * @param in The data received
 * @param len The length in bytes of the received data
 * @return 0 for success, otherwise -1
 */
int callback_info(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len) {
	enum InfoCommand c;
	struct toSend *s;
	int res;
	char *videoName;

	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		res = 0;
		c = getInfoCommand(strtok(in, "\t"));
		s = (struct toSend *) user;
		switch (c) {
		case LIST_VIDEOS:
			fprintf(stderr, "listing videos\n");
			s->writeMode = LWS_WRITE_TEXT;
			s->data = getVideoList(&s->size);
			if (s->data == NULL) {
				fprintf(stderr, "Could not get video list!\n");
			}
			break;
		case GET_MPD:
			fprintf(stderr, "sending mpd\n");
			s->writeMode = LWS_WRITE_BINARY;
			videoName = strtok(NULL, "\t");
			s->data = getInfoFile(videoName, MPD_NAME, &s->size);
			if (s->data == NULL) {
				fprintf(stderr, "Could not get mpd file!\n");
			}
			break;
		default:
			fprintf(stderr, "Unknown command received!\n");
			break;
		}

		if (s->data == NULL) {
			break;
		}

		if (s->size < RX_BUFFER_SIZE) {
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
		if (s->size - s->sent > RX_BUFFER_SIZE) {
			res = libwebsocket_write(wsi, &s->data[LWS_SEND_BUFFER_PRE_PADDING + s->sent], RX_BUFFER_SIZE, s->writeMode | LWS_WRITE_NO_FIN);
			fprintf(stderr, "send res: %d\n", res);
			if (res < 0) {
				free(s->data);
				s->data = NULL;
				s->size = 0;
			} else {
				s->writeMode = LWS_WRITE_CONTINUATION;
				s->sent += RX_BUFFER_SIZE;
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

