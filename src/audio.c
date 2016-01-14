#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "file_utilities.h"
#include "intern.h"
#include "audio.h"

const int nrAudioCommands = 2;
const char *audioCommandStr[] = { "ini", "get" };
enum AudioCommand {
	GET_INIT = 0, GET_DATA_SEG = 1, UNKNOWN = 2
};

/**
 * Converts an audio command string to the corresponding AudioCommand enum
 * @param command The command string
 * @return The corresponding AudioCommand enum
 */
enum AudioCommand getAudioCommand(char *in) {
	int i;
	for (i = 0; i < nrAudioCommands; i++) {
		if (strcmp(in, audioCommandStr[i]) == 0) {
			break;
		}
	}
	return (enum AudioCommand) i;
}

/**
 * Callback for the "audio" websockets protocol
 * @param ctx The context
 * @param wsi The client connection
 * @param reason The reason for the callback
 * @param user The allocated per user data
 * @param in The data received
 * @param len The length in bytes of the received data
 * @return 0 for success, otherwise -1 (sends a close signal to the client)
 */
int callback_audio(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len) {
	enum AudioCommand c;
	struct toSend *s;
	int res;
	char *videoName;
	char *segStr;
	int segNr;

	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		res = 0;
		c = getAudioCommand(strtok(in, "\t"));
		s = (struct toSend *) user;

		switch (c) {
		case GET_INIT:
			fprintf(stderr, "sending audio init\n");
			s->writeMode = LWS_WRITE_BINARY;
			videoName = strtok(NULL, "\t");
			s->data = getInfoFile(videoName, AUDIO_INIT_NAME, &s->size);
			if (s->data == NULL) {
				fprintf(stderr, "Could not get audio init!\n");
			}
			break;
		case GET_DATA_SEG:
			videoName = strtok(NULL, "\t");
			segStr = strtok(NULL, "\t");
			res = str2int(segStr, &segNr);

			if (res == 0) {
				s->data = getEncodedSeg(videoName, segStr, AUDIO, DATA, &s->size);
				if (s->data == NULL) {
					s->data = (unsigned char *) getRedirect(segNr);
					if (s->data == NULL) {
						fprintf(stderr, "No such audio seg: d%s\n", segStr);
					} else {
						s->size = strlen((char *) &s->data[LWS_SEND_BUFFER_PRE_PADDING]);
						s->writeMode = LWS_WRITE_TEXT;
						fprintf(stderr, "Redirecting to: %s\n", &s->data[LWS_SEND_BUFFER_PRE_PADDING]);
					}
				} else {
					s->writeMode = LWS_WRITE_BINARY;
					fprintf(stderr, "sending audio seg %s\n", segStr);
				}
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
			if (res < 0) {
				fprintf(stderr, "send res: %d\n", res);
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
