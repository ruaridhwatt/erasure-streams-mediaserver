#include "upload.h"

int send_message(void *message, int length, int write_mode, struct libwebsocket *wsi) {
	if (wsi != NULL) {
		int paddingSize = LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING;
		unsigned char *buff = (unsigned char *) malloc(paddingSize + length);
		memcpy(&(buff[LWS_SEND_BUFFER_PRE_PADDING]), message, length);

		int res = libwebsocket_write(wsi, &buff[LWS_SEND_BUFFER_PRE_PADDING], length, write_mode);

		if(res < 0) {
			fprintf(stderr, "erros sending, %d!\n", res);
		}

      free(buff);
      return EXIT_SUCCESS;
   }
   return EXIT_FAILURE;
}

void send_video_list(struct libwebsocket_context *ctx, struct libwebsocket *wsi) {
	pthread_mutex_lock(&mutex);
	element *position = llist_first(videoList);
	char *tempString1 = malloc((sizeof(char) * strlen("video-list")) + 1);
	strcpy(tempString1, "video-list");
	char *tempString2;
	while(!llist_isEnd(position)){
		tempString2 = stringAppender(tempString1, "\t");
		free(tempString1);
		tempString1 = stringAppender(tempString2, (char*)llist_inspect(position));
		free(tempString2);
		position = llist_next(position);
	}
	pthread_mutex_unlock(&mutex);
	send_message((char*)tempString1, strlen(tempString1) + 1, LWS_WRITE_TEXT, wsi);
	free(tempString1);
}

void send_mpd(struct libwebsocket_context *ctx, struct libwebsocket *wsi, struct info_user * thisUser) {
	fprintf(stderr, "filedir: %s\n", thisUser->mpd_filedir);
	if(access(thisUser->mpd_filedir, F_OK ) != -1) {
		FILE *fileptr = fopen(thisUser->mpd_filedir, "rb");
		fseek(fileptr, 0, SEEK_END);
		long filelen = ftell(fileptr);
		rewind(fileptr);

		unsigned char *buffer = (unsigned char *)malloc((PAYLOAD)*sizeof(unsigned char));
		fread(buffer, 1, filelen, fileptr);
		send_message(buffer, filelen, LWS_WRITE_BINARY, wsi);
		fclose(fileptr);
		free(buffer);
	} else {
		send_message(NOK, strlen(NOK), LWS_WRITE_TEXT, wsi);
	}
}

void putVideoInList(char *filename) {
	element *currentElement = llist_first(videoList);
	char* video = malloc((sizeof(char) * strlen(filename)) + 1);
	strcpy(video, filename);
	llist_insert(currentElement, videoList, (char*)video);
}

void putVideoInMap(char *filename) {
	entry *mentry = create_entry(filename, "localhost");
	hashmap_put(mentry, videoMap);
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
		char *shortendFileN = noMp4(user->filename);
		char *tempString1 = stringAppender(shortendFileN, "/");
		char *filestr = stringAppender(tempString1, user->filename);

		FILE *received_file = fopen(filestr, "a+b");
		free(tempString1);
		free(shortendFileN);
		free(filestr);
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

	char *tempString1 = stringAppender(BENTOSCRIPT, fileName);
	char *tempString2 = stringAppender(tempString1, " ");
	free(tempString1);
	tempString1 = stringAppender(tempString2, k);
	free(tempString2);
	tempString2 = stringAppender(tempString1, " ");
	free(tempString1);
	char *bentoCmd = stringAppender(tempString2, m);

	int res = system(bentoCmd);
	if(res < 0) {
		fprintf(stderr, "Failed to run %s\n", bentoCmd);
		exit(EXIT_FAILURE);
	}

	pthread_mutex_lock(&mutex);
	putVideoInList(fileName);
	pthread_mutex_unlock(&mutex);

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
}

int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {

	struct upload_user * thisUser = (struct upload_user *) user;
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
			char *inString = (char*)in;
			if(startsWith(START_UPLOAD, inString)) {
				inString += strlen(START_UPLOAD) + 1;
				printf((char*) in);
				thisUser->grantedUpload = true;
				thisUser->uploadComplete = false;
				thisUser->filename = (char *)malloc((strlen(inString) + 1)*sizeof(char));
				strcpy(thisUser->filename, inString);
				fprintf(stderr, "FileName: %s\n", thisUser->filename);
				libwebsocket_callback_on_writable(ctx, wsi);
			}
		}
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if(thisUser->grantedUpload && (hashmap_get(thisUser->filename, strlen(thisUser->filename) + 1, videoMap) == NULL)) {
			char *shortendFileN = noMp4(thisUser->filename);
			if( access( shortendFileN, F_OK ) != -1 ) {
				send_message(NOK, strlen(NOK), LWS_WRITE_TEXT, wsi);
			} else {
				char *mkdir = stringAppender("mkdir ", shortendFileN);
				int res = system(mkdir);
				if(res < 0) {
					fprintf(stderr, "Failed to run mkdir\n");
					exit(EXIT_FAILURE);
				}
				free(mkdir);

				send_message(OK, strlen(OK), LWS_WRITE_TEXT, wsi);
				pthread_mutex_lock(&mutex);
				putVideoInMap(thisUser->filename);
				pthread_mutex_unlock(&mutex);
				fprintf(stderr, "skickat ok\n");
				free(shortendFileN);
			}
		} else {
			thisUser->grantedUpload = false;
			send_message(NOK, strlen(NOK), LWS_WRITE_TEXT, wsi);
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

int callback_info(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len) {

	struct info_user * thisUser = (struct info_user *) user;
	switch (reason) {
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		break;
	case LWS_CALLBACK_ESTABLISHED:
		printf("Client info connection established\n");

		break;
	case LWS_CALLBACK_RECEIVE:
		if (!lws_frame_is_binary(wsi)) {
			char *inString = (char*)in;
			if(startsWith(LIST_STREAMS, inString)) {
				thisUser->cmd = list;
				libwebsocket_callback_on_writable(ctx, wsi);

			} else if (startsWith(MPD, inString)){
				inString += strlen(MPD) + 1;
				if((hashmap_get(inString, strlen(inString) + 1, videoMap) != NULL)) {
					thisUser->cmd = mpd;
					removeSubstring(inString, ".mp4");
					thisUser->mpd_filedir = stringAppender(inString, "/mpd");
					libwebsocket_callback_on_writable(ctx, wsi);
				}
			}
		}
		break;
	case LWS_CALLBACK_CLOSED:
		printf("Connection Closed\n");
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		switch(thisUser->cmd) {
		case list:
			send_video_list(ctx, wsi);
			break;
		case mpd:
			send_mpd(ctx, wsi, thisUser);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return 0;
}
