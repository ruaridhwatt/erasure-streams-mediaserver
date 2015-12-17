#include "upload.h"

char *appendString(char *s1, char *s2) {
	char *s3 = (char *)malloc((strlen(s1) + strlen(s2) + 2)*sizeof(char));
	strcpy(s3, s1);
	strcat(s3, s2);
	return s3;
}

void removeMp4(char *fileName) {
	while((fileName=strstr(fileName,".mp4")))
		memmove(fileName,fileName+strlen(".mp4"),1+strlen(fileName+strlen(".mp4")));
}

void receiveFile(void *in, size_t len, struct upload_user *user) {
	if (len == 1 && *((unsigned char *) in) == 0) {
		if (user->terminatorReceived == true) {
			/* Second terminator received */
			printf("Upload done!\n");
			user->grantedUpload = false;
			user->terminatorReceived = false;
			user->uploadComplete = true;
			/* TODO start distribution */
		}
		user->terminatorReceived = true;
	} else {
		printf("%d\n", (int)len);
		FILE *received_file = fopen(user->mp4Dir, "a+b");

		if (received_file == NULL) {
			fprintf(stderr, "Failed to open file %s\n", user->filename);
			exit(EXIT_FAILURE);
		}

		if (user->terminatorReceived == true) {
			user->terminatorReceived = false;
			/* Write terminator that wasn't! */
			unsigned char pseudoTerm = 0;
			fwrite(&pseudoTerm, sizeof(unsigned char), 1, received_file);
		}

		fwrite(in, sizeof(unsigned char), len, received_file);
		fclose(received_file);
	}
}

void *initBentoFragmention(void *fName) {

	char *fileName = (char*)fName;
	fprintf(stderr, "bento Filename: \n%s\n", fileName);

	char *tempString1 = appendString(BENTOSCRIPT, fileName);
	char *tempString2 = appendString(tempString1, " ");
	free(tempString1);
	tempString1 = appendString(tempString2, k);
	free(tempString2);
	tempString2 = appendString(tempString1, " ");
	free(tempString1);
	char *bentoCmd = appendString(tempString2, m);

	fprintf(stderr, "bentoCmd: %s\n", bentoCmd);
	int res = system(bentoCmd);
	if(res < 0) {
		fprintf(stderr, "Failed to run %s\n", bentoCmd);
		exit(EXIT_FAILURE);
	}

	free(bentoCmd);
	free(fileName);

	printf("fraggy done!\n");
	return NULL;
}

void uploadComplete(struct libwebsocket_context * ctx, struct upload_user *thisUser) {
	pthread_t thread;
	char *fileName = (char *)malloc((strlen(thisUser->filename) + 1)*sizeof(char));
	strcpy(fileName, thisUser->filename);
	int res; res = pthread_create(&thread, NULL, initBentoFragmention, fileName);
	if (res){
		perror("pthread_create");
		libwebsocket_context_destroy(ctx);
		fprintf(stderr, "Exiting...\n");
		exit(EXIT_FAILURE);
	}
	thisUser->uploadComplete = false;
	free(thisUser->filename);
	free(thisUser->dir);
	free(thisUser->dotDir);
	free(thisUser->mp4Dir);
}

UPL_CMDS getUPLCommand(char *in) {

	if(strncmp(START_UPLOAD, in, strlen(START_UPLOAD)) == 0) {
		return UPL;
	}
	return UNKNOWN;
}

int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {

	struct upload_user * thisUser = (struct upload_user *) user;
	UPL_CMDS c;
	char *videoDir;
	int paddingSize = LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING;
	unsigned char *buff;
	int res;
	switch (reason) {

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		printf("Handshake failed\n");
		break;

	case LWS_CALLBACK_ESTABLISHED:
		printf("Connection upload established\n");
		thisUser->grantedUpload = false;
		thisUser->terminatorReceived = false;
		thisUser->uploadComplete = false;
		break;

	case LWS_CALLBACK_RECEIVE:

		if (lws_frame_is_binary(wsi) && thisUser->grantedUpload) {
			receiveFile(in, len, thisUser);
			if(thisUser->uploadComplete) {
				uploadComplete(ctx, thisUser);
			}

		} else if(!lws_frame_is_binary(wsi)) {
			c = getUPLCommand(in);
			videoDir = getenv("VIDEO_DIR");

			switch (c) {
			case UPL:
				in += strlen(START_UPLOAD) + 1;

				thisUser->filename = (char *)malloc((len + 1)*sizeof(char));

				strcpy(thisUser->filename, in);
				thisUser->dir = appendString(videoDir, thisUser->filename);
				removeMp4(thisUser->dir);

				char *dotFileName = appendString(".", thisUser->filename);
				thisUser->dotDir = appendString(videoDir, dotFileName);
				removeMp4(thisUser->dotDir);

				char *tempString = appendString(thisUser->dotDir, "/");
				thisUser->mp4Dir = appendString(tempString, thisUser->filename);

				if((access( thisUser->dir, F_OK ) != -1 ) || (access( thisUser->dotDir, F_OK ) != -1 )) {

					buff = (unsigned char *) malloc(paddingSize + 3);
					memcpy(&(buff[LWS_SEND_BUFFER_PRE_PADDING]), "NOK", 3);
					res = libwebsocket_write(wsi, &buff[LWS_SEND_BUFFER_PRE_PADDING], 3, LWS_WRITE_TEXT);

				} else {
					thisUser->grantedUpload = true;
					thisUser->uploadComplete = false;
					char *mkdir = appendString("mkdir ", thisUser->dotDir);
					int sysres = system(mkdir);
					if(sysres < 0) {
						fprintf(stderr, "Failed to run mkdir\n");
						free(mkdir);
						exit(EXIT_FAILURE);
					}
					free(mkdir);
					free(dotFileName);

					buff = (unsigned char *) malloc(paddingSize + 2);
					memcpy(&(buff[LWS_SEND_BUFFER_PRE_PADDING]), "OK", 2);
					res = libwebsocket_write(wsi, &buff[LWS_SEND_BUFFER_PRE_PADDING], 2, LWS_WRITE_TEXT);
				}

				if(res < 0) {
					fprintf(stderr, "lws_write < 0\n");
				}

				free(tempString);
				free(dotFileName);
				break;
			default:
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
	return 0;
}
