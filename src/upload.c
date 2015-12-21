#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include "upload.h"

char *appendString(char *s1, char *s2) {
	char *s3 = (char *) malloc((strlen(s1) + strlen(s2) + 2) * sizeof(char));
	strcpy(s3, s1);
	strcat(s3, s2);
	return s3;
}

void removeMp4(char *fileName) {
	char *pos;
	pos = index(fileName, '.');
	if (pos != NULL) {
		*pos = '\0';
	}
}

int receiveFile(void *in, size_t len, struct upload_user *user) {
	int res;
	FILE *received_file;
	unsigned char pseudoTerm;

	res = 0;
	if (len == 1 && *((unsigned char *) in) == 0) {
		if (user->terminatorReceived == true) {
			/* Second terminator received */
			printf("Upload done!\n");
			user->grantedUpload = false;
			user->terminatorReceived = false;
			user->uploadComplete = true;
			/* TODO start distribution */

		} else {
			user->terminatorReceived = true;
		}
	} else {
		printf("%d\n", (int) len);
		received_file = fopen(user->mp4Dir, "a+b");

		if (received_file == NULL) {
			fprintf(stderr, "Failed to open file %s\n", user->filename);
			res = -1;
		} else {
			if (user->terminatorReceived == true) {
				user->terminatorReceived = false;
				/* Write terminator that wasn't! */
				pseudoTerm = 0;
				fwrite(&pseudoTerm, sizeof(unsigned char), 1, received_file);
			}
			fwrite(in, sizeof(unsigned char), len, received_file);
			fclose(received_file);
		}
	}
	return res;
}

void *initBentoFragmention(void *filename) {
	int res;
	char *bentoCmd, *pos;

	fprintf(stderr, "bento Filename: \n%s\n", (char *) filename);

	bentoCmd = (char *) malloc(strlen(BENTOSCRIPT) + 1 + strlen(filename) + 1 + strlen(kStr) + 1 + strlen(mStr) + 1);
	if (bentoCmd == NULL) {
		return NULL;
	}

	pos = bentoCmd;
	strcpy(pos, BENTOSCRIPT);
	pos += strlen(BENTOSCRIPT);
	*pos = ' ';
	pos++;
	strcpy(pos, filename);
	pos += strlen(filename);
	free(filename);
	*pos = ' ';
	pos++;
	strcpy(pos, kStr);
	pos += strlen(kStr);
	*pos = ' ';
	pos++;
	strcpy(pos, mStr);
	pos += strlen(mStr);
	*pos = '\0';

	fprintf(stderr, "bentoCmd: %s\n", bentoCmd);
	res = system(bentoCmd);
	free(bentoCmd);
	if (res < 0) {
		fprintf(stderr, "Failed to fragment uploaded video\n");
	}
	return NULL;
}

void uploadComplete(struct upload_user *thisUser) {
	pthread_t thread;
	char *fileName;
	int res;

	fileName = (char *) malloc((strlen(thisUser->filename) + 1) * sizeof(char));
	strcpy(fileName, thisUser->filename);

	res = pthread_create(&thread, NULL, initBentoFragmention, fileName);
	if (res != 0) {
		/* Don't stop! People can still stream iaf */
		perror("pthread_create");
	}
	thisUser->uploadComplete = false;
	free(thisUser->filename);
	free(thisUser->dir);
	free(thisUser->dotDir);
	free(thisUser->mp4Dir);
}

UPL_CMDS getUPLCommand(char *in) {

	if (strcmp(START_UPLOAD, in) == 0) {
		return UPL;
	}
	return UNKNOWN;
}

int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len) {

	struct upload_user *thisUser = (struct upload_user *) user;
	UPL_CMDS c;
	char *videoDir, *filename, *pos, *temp;
	int paddingSize = LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING;
	unsigned char *buff;
	int res;

	res = 0;
	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		printf("Connection upload established\n");
		thisUser->grantedUpload = false;
		thisUser->terminatorReceived = false;
		thisUser->uploadComplete = false;
		break;

	case LWS_CALLBACK_RECEIVE:
		if (lws_frame_is_binary(wsi) && thisUser->grantedUpload) {
			receiveFile(in, len, thisUser);
			if (thisUser->uploadComplete) {
				uploadComplete(thisUser);
			}
		} else if (!lws_frame_is_binary(wsi)) {
			c = getUPLCommand(strtok(in, "\t"));
			videoDir = getenv("VIDEO_DIR");

			switch (c) {
			case UPL:
				filename = strtok(NULL, "\t");
				fprintf(stderr, "%s\n", filename);
				thisUser->filename = (char *) malloc((strlen(filename) + 1) * sizeof(char));
				strcpy(thisUser->filename, filename);

				pos = index(filename, '.');
				if (pos != NULL) {
					*pos = '\0';
				}
				thisUser->dir = appendString(videoDir, thisUser->filename);
				fprintf(stderr, "%s\n", thisUser->dir);

				temp = appendString(".", filename);
				thisUser->dotDir = appendString(videoDir, temp);
				free(temp);
				fprintf(stderr, "%s\n", thisUser->dotDir);

				temp = appendString(thisUser->dotDir, "/");
				thisUser->mp4Dir = appendString(temp, thisUser->filename);
				free(temp);
				fprintf(stderr, "%s\n", thisUser->mp4Dir);

				if ((access(thisUser->dir, F_OK) != -1) || (access(thisUser->dotDir, F_OK) != -1)
						|| (mkdir(thisUser->dotDir, S_IRWXU | S_IRWXG) < 0)) {

					free(thisUser->filename);
					free(thisUser->dir);
					free(thisUser->dotDir);
					free(thisUser->mp4Dir);
					buff = (unsigned char *) malloc(paddingSize + 4);
					strcpy(&(buff[LWS_SEND_BUFFER_PRE_PADDING]), "NOK");
					res = libwebsocket_write(wsi, &buff[LWS_SEND_BUFFER_PRE_PADDING], 3, LWS_WRITE_TEXT);
					free(buff);

				} else {
					thisUser->grantedUpload = true;
					thisUser->uploadComplete = false;
					buff = (unsigned char *) malloc(paddingSize + 3);
					strcpy(&(buff[LWS_SEND_BUFFER_PRE_PADDING]), "OK");
					res = libwebsocket_write(wsi, &buff[LWS_SEND_BUFFER_PRE_PADDING], 2, LWS_WRITE_TEXT);
					free(buff);
				}
				if (res < 0) {
					fprintf(stderr, "Failed to send message!\n");
				}
				break;
			default:
				fprintf(stderr, "Unknown command received");
				break;
			}
		}
		break;
	case LWS_CALLBACK_CLOSED:
		printf("Connection Closed\n");
		break;
	default:
		break;
	}
	return res;
}
