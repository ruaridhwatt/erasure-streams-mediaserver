/*
 * erasure_Server.h
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
#include <getopt.h>

#include "global_variables.h"
#include "stream_video_audio.h"
#include "upload.h"

static volatile int force_exit = 0;
static struct libwebsocket_context *context;

static int callback_video(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len);
static int callback_audio(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len);
void sighandler(int sig);

static struct libwebsocket_protocols protocols[] = {
		{"upload", callback_upload, sizeof(struct upload_user), UPLOAD, 0},
		{"info", callback_info, sizeof(struct info_user) , PAYLOAD, 0},
		{"audio", callback_audio, sizeof(struct per_session_data) , PAYLOAD, 0},
		{"video", callback_video, sizeof(struct per_session_data) , PAYLOAD, 0}, { NULL, NULL, 0 }};
