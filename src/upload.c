#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include "file_utilities.h"
#include "upload.h"

const int nrUploadCommands = 2;
const char *uploadCommandStr[] = { "upl", "fin" };

enum UploadCommand {
	INITIALIZE_UPLOAD = 0, UPLOAD_FINNISHED = 1, UNKNOWN = 2
};

enum UploadCommand getUploadCommand(char *in) {

	int i;
	for (i = 0; i < nrUploadCommands; i++) {
		if (strcmp(in, uploadCommandStr[i]) == 0) {
			break;
		}
	}
	return (enum UploadCommand) i;
}

int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len) {

	struct upload_user *thisUser = (struct upload_user *) user;
	enum UploadCommand c;
	char *inStr, *commandStr, *filename;
	size_t written;
	int res;

	res = 0;
	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		thisUser->f = NULL;
		thisUser->filename = NULL;
		break;

	case LWS_CALLBACK_RECEIVE:
		if (lws_frame_is_binary(wsi)) {
			if (thisUser->f == NULL) {
				res = -1;
				break;
			}
			written = fwrite(in, sizeof(unsigned char), len, thisUser->f);
			if (written != len) {
				freeIncompleteUpload(thisUser->filename);
				res = -1;
			}
		} else {
			inStr = (char *) in;
			if (inStr[len] != '\0') {
				fprintf(stderr, "Non null terminated string received!");
				res = -1;
				break;
			}
			commandStr = strtok(in, "\t");
			c = getUploadCommand(commandStr);
			switch (c) {
			case INITIALIZE_UPLOAD:
				filename = strtok(NULL, "\t");
				thisUser->f = prepUpload(filename);
				if (thisUser->f == NULL) {
					send(NACK, wsi);
					break;
				}
				thisUser->filename = (char *) malloc(strlen(filename) * sizeof(char));
				strcpy(thisUser->filename, filename);
				send(ACK, wsi);
				break;
			case UPLOAD_FINNISHED:
				if (thisUser->f != NULL) {
					fclose(thisUser->f);
					res = startFragmentation(thisUser->filename);
					if (res != 0) {
						freeIncompleteUpload(thisUser->filename);
					}
				} else {
					res = -1;
				}
				break;
			default:
				fprintf(stderr, "Unknown command received: %s", commandStr);
				res = -1;
				break;
			}
		}
		break;
	case LWS_CALLBACK_CLOSED:
		if (thisUser->f != NULL) {
			fclose(thisUser->f);
		}
		if (thisUser->filename != NULL) {
			freeIncompleteUpload(thisUser->filename);
		}
		printf("Connection Closed\n");
		break;
	default:
		break;
	}

	if (res != 0) {
		if (thisUser->f != NULL) {
			fclose(thisUser->f);
		}
		if (thisUser->filename != NULL) {
			freeIncompleteUpload(thisUser->filename);
		}
	}
	return res;
}

int send(char *string, struct libwebsocket *wsi) {
	int bufsize, res;
	char *buf;

	bufsize = LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING;
	bufsize += strlen(string) + 1;
	buf = (char *) malloc(bufsize * sizeof(char));
	if (buf == NULL) {
		return -1;
	}
	strcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], string);
	res = libwebsocket_write(wsi, (unsigned char *) buf, strlen(string), LWS_WRITE_TEXT);
	free(buf);
	return res;
}
