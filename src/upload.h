/*
 * upload.h
 *
 *  Created on: Dec 16, 2015
 *      Author: dv11ann
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <unistd.h>

#include "global_variables.h"
#include "llist.h"
#include "hashmap.h"
#include "hashmapSettings.h"
#include "string_functions.h"

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

char *k;
char *m;
pthread_mutex_t mutex;
hashmap *videoMap;
llist *videoList;

int send_message(void *message, int length, int write_mode, struct libwebsocket *wsi);
void send_video_list(struct libwebsocket_context *ctx, struct libwebsocket *wsi);
void send_mpd(struct libwebsocket_context *ctx, struct libwebsocket *wsi, struct info_user * thisUser);
void putVideoInList(char *filename);
void putVideoInMap(char *filename);
void receiveFile(void *in, size_t len, struct upload_user *user);
void *initBentoFragmention(void *fName);
void uploadComplete(struct libwebsocket_context * ctx, struct upload_user *thisUser);
int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);
int callback_info(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len);



