/*
 ============================================================================
 Name        : ErasureServer.c
 ============================================================================
 Name        : ErasureServer.c
 Author      : dv11ann, dv12rwt
 Version     : 0.0
 Copyright   : Your copyright notice
 Description : ErasureServer
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <unistd.h>
#include <getopt.h>
#include "llist.h"
#include "hashmap.h"
#include "hashmapSettings.h"
#include "string_functions.h"

#define PAYLOAD 10*1024
#define UPLOAD 1024*1024

#define OK "OK"
#define NOK "NOK"
#define SWITCH_SERVER "switch-server"

#define EOA "EOA"
#define EOV "EOV"
#define EOS "EOS"

#define INI "ini"
#define MPD "mpd"
#define GET "get"
#define TAB "\t"

#define START_UPLOAD "upl"
#define LIST_STREAMS "lst"

#define CMD_SIZE 3
#define BENTOSCRIPT "scripts/BentoHandleScript.sh "
#define FILEHANDLESCRIPT "./FileHandleScript.sh "

int received = 0;
char *k;
char *m;

static volatile int force_exit = 0;
static struct libwebsocket_context *context;
hashmap *videoMap;
llist *videoList;
pthread_mutex_t mutex;

struct upload_user {
   bool grantedUpload;
   bool terminatorReceived;
   bool uploadComplete;
   char *filename;
};

typedef enum {mpd, list} cmd_type;

struct info_user {
	cmd_type cmd;
	char *mpd_filedir;
};

typedef enum {ini, seg} file_type;

struct per_session_data {
	long total_packets;
	long package_to_send;
	long filesize;
	int sendsize;
	unsigned char *buff;
	char *filedir;
	int isSendingFile;
	bool nok;
};

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
	fprintf(stderr,"received: %d\n",received);
	thisUser->uploadComplete = false;
	free(thisUser->filename);
}

static int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi,
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
			received++; //ta bort
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

void setTotalFragments(struct per_session_data *psd) {
	long slicenr = 0;
	long remain = psd->filesize;
	while (remain) {
		long toCpy = remain > PAYLOAD-5 ? PAYLOAD-5 : remain;
		remain -= toCpy;
		slicenr++;
	}
	psd->total_packets = slicenr;
}

void initFileInfo(struct per_session_data *psd) {
	if( access( psd->filedir, F_OK ) != -1 ) {
		FILE *fileptr = fopen(psd->filedir, "rb");
		fseek(fileptr, 0, SEEK_END);
		long filelen = ftell(fileptr);
		rewind(fileptr);
		psd->filesize = filelen;
		setTotalFragments(psd);
		fprintf(stderr, "file read success %s\n", psd->filedir);
		fclose(fileptr);
	} else {
		fprintf(stderr, "fileread fail\n");
	    psd->nok = true;
	}
}

unsigned char *getFragment(struct per_session_data *psd) {
	fprintf(stderr, "filedir: %s\n", psd->filedir);
	FILE *fileptr = fopen(psd->filedir, "rb");
	long remain =psd->filesize;
	long slicenr = 0;
	rewind(fileptr);

	unsigned char *buffer = (unsigned char *)malloc((PAYLOAD)*sizeof(unsigned char));
	while (remain) {
		long toCpy = remain > PAYLOAD ? PAYLOAD : remain;
		remain -= toCpy;
		fread(buffer, 1, toCpy, fileptr);
		if(slicenr == psd->package_to_send) {
			psd->sendsize = toCpy;
			break;
		}
		slicenr++;
	}
	fprintf(stderr, "sendsize: %d\n", psd->sendsize);
	fclose(fileptr);
	return buffer;

}

void stream_audio_video(char* in, struct per_session_data *psd, struct libwebsocket_context *ctx, struct libwebsocket *wsi, bool isVideo) {
	char *fileName = (char*)in;

	if(startsWith(INI, fileName)) {
		fileName += strlen(INI) + 1;
		if((hashmap_get(fileName, strlen(fileName) + 1, videoMap) != NULL)) {
			removeSubstring(fileName, ".mp4");
			if(isVideo) {
				psd->filedir = stringAppender(fileName, "/vinit");
			} else {
				psd->filedir = stringAppender(fileName, "/ainit");
			}
			psd->package_to_send = 0;
			psd->nok = false;
			fprintf(stderr, "filedir: %s\n", psd->filedir);
			initFileInfo(psd);
			libwebsocket_callback_on_writable(ctx, wsi);
		} else {
			psd->nok = true;
			libwebsocket_callback_on_writable(ctx, wsi);
		}
	} else if(startsWith(GET, fileName)) {
		fileName += strlen(GET) + 1;
		strtok(fileName, TAB);
		char *sliceNr =  strtok(NULL, TAB);
		if((hashmap_get(fileName, strlen(fileName) + 1, videoMap) != NULL)) {
			char *dir;
			if(isVideo) {
				dir = setVideoDirectory(fileName);
			} else {
				dir = setAudioDirectory(fileName);
			}

			char *tempString1 = stringAppender(dir, "d");
			psd->filedir = stringAppender(tempString1, sliceNr);
			psd->package_to_send = 0;
			fprintf(stderr, "filedir: %s\n", psd->filedir);

			free(tempString1);
			free(dir);
			psd->nok = false;
			initFileInfo(psd);
			libwebsocket_callback_on_writable(ctx, wsi);
		} else {
			psd->nok = true;
			libwebsocket_callback_on_writable(ctx, wsi);
		}
	}
}

static int callback_video(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len) {

	struct per_session_data *psd = user;
	unsigned char* buf;
	switch (reason) {

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		libwebsocket_rx_flow_control (wsi, 1);
		break;
	case LWS_CALLBACK_ESTABLISHED:
		printf("Client video connection established\n");
		psd->isSendingFile = 0;
		break;
	case LWS_CALLBACK_RECEIVE:
		fprintf(stderr, "video\n");
		fprintf(stderr, "in: %s\n", (char*)in);
		if (!lws_frame_is_binary(wsi)) {
			stream_audio_video((char*)in, psd, ctx, wsi, true);
		}
		break;
	case LWS_CALLBACK_CLOSED:
		printf("Connection video Closed\n");
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if(psd->nok) {
			fprintf(stderr, "skickar nok\n");
			send_message(NOK, strlen(NOK), LWS_WRITE_TEXT, wsi);
			free(psd->filedir);
		} else if(psd->package_to_send < psd->total_packets) {
			buf = getFragment(psd);
			send_message(buf, psd->sendsize, LWS_WRITE_BINARY, wsi);
			free(buf);
			psd->package_to_send = psd->package_to_send + 1;
			libwebsocket_callback_on_writable(ctx, wsi);
		} else {
			fprintf(stderr, "video sending EOS\n");
			send_message(EOS, strlen(EOS), LWS_WRITE_TEXT, wsi);
			free(psd->filedir);
		}
		break;
	default:
		break;
	}
	return 0;
}

static int callback_audio(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len) {

	struct per_session_data *psd = user;
	unsigned char* buf;
	switch (reason) {

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		libwebsocket_rx_flow_control (wsi, 1);
		break;
	case LWS_CALLBACK_ESTABLISHED:
		printf("Client audio connection established\n");
		psd->isSendingFile = 0;
		break;
	case LWS_CALLBACK_RECEIVE:
		fprintf(stderr, "uadio\n");
		if (!lws_frame_is_binary(wsi)) {
			stream_audio_video((char*)in, psd, ctx, wsi, false);
		}
		break;
	case LWS_CALLBACK_CLOSED:
		printf("Connection audio Closed\n");
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if(psd->nok) {
			fprintf(stderr, "skickar nok\n");
			send_message(NOK, strlen(NOK), LWS_WRITE_TEXT, wsi);
			free(psd->filedir);
		} else if(psd->package_to_send < psd->total_packets) {
			buf = getFragment(psd);
			send_message(buf, psd->sendsize, LWS_WRITE_BINARY, wsi);
			free(buf);
			psd->package_to_send = psd->package_to_send + 1;
			libwebsocket_callback_on_writable(ctx, wsi);
		} else {
			fprintf(stderr, "audio sending EOS\n");
			send_message(EOS, strlen(EOS), LWS_WRITE_TEXT, wsi);
			free(psd->filedir);
		}
		break;
	default:
		break;
	}
	return 0;
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
	FILE *fileptr = fopen(thisUser->mpd_filedir, "rb");
	fseek(fileptr, 0, SEEK_END);
	long filelen = ftell(fileptr);
	rewind(fileptr);

	unsigned char *buffer = (unsigned char *)malloc((PAYLOAD)*sizeof(unsigned char));
	fread(buffer, 1, filelen, fileptr);
	send_message(buffer, filelen, LWS_WRITE_BINARY, wsi);
	fclose(fileptr);
	free(buffer);
}

static int callback_info(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
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
					char *dir = setMPDDirectory(inString);
					removeSubstring(inString, ".mp4");
					thisUser->mpd_filedir = stringAppender(inString, "/mpd");
					free(dir);
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

static struct libwebsocket_protocols protocols[] = {
		{"upload", callback_upload, sizeof(struct upload_user), UPLOAD, 0},
		{"info", callback_info, sizeof(struct info_user) , PAYLOAD, 0},
		{"audio", callback_audio, sizeof(struct per_session_data) , PAYLOAD, 0},
		{"video", callback_video, sizeof(struct per_session_data) , PAYLOAD, 0}, { NULL, NULL, 0 }};

void sighandler(int sig) {
	force_exit = 1;
}

int main(int argc, char *argv[]) {
	signal(SIGINT, sighandler);
	int PORT;

	int c;

	int kflag = 0;
	int mflag = 0;
	int pflag = 0;
	while ((c = getopt (argc, argv, "k:m:p:")) != -1) {
		switch(c) {
		case 'k':
			kflag = 1;
			k = malloc(sizeof(char) * (strlen(optarg) + 1));
			strcpy(k, optarg);
			break;
		case 'm':
			mflag = 1;
			m = malloc(sizeof(char) * (strlen(optarg) + 1));
			strcpy(m, optarg);
			break;
		case 'p':
			pflag = 1;
			PORT = atoi(optarg);
			break;
		case '?':
			fprintf(stderr, "./Server -k <k> -m <m> -p <port>\n");
			exit(1);
			break;
		default:
			fprintf(stderr, "./Server -k <k> -m <m> -p <port>\n");
			exit(1);
		}
	}

	if(kflag == 0 || mflag == 0 || pflag == 0) {
		fprintf(stderr, "./Server -k <k> -m <m> -p <port>\n");
		exit(1);
	}

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));
	info.port = PORT;
	info.gid = -1;
	info.uid = -1;
	info.protocols = protocols;

	videoMap = hashmap_empty(20, string_hash_function, entry_free_func);
	videoList = llist_empty(free);
	//putVideoInList("test.mp4");
	//putVideoInMap("test.mp4");
	pthread_mutex_init(&mutex, NULL);

	printf("starting server...\n");
   context = libwebsocket_create_context(&info);
   if (context == NULL) {
      fprintf(stderr, "libwebsocket init failed\n");
      return EXIT_FAILURE;
   }

   while (!force_exit) {
      libwebsocket_service(context, 500);
   }
   libwebsocket_context_destroy(context);
   free(k);
   free(m);
   llist_free(videoList);
   hashmap_free(videoMap);
   return EXIT_SUCCESS;
}
