/*
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
#include <string.h>
#include <signal.h>
#include <libwebsockets.h>

static volatile int force_exit = 0;
static struct libwebsocket_context *context;

static int callback_init(struct libwebsocket_context * ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len) {

	switch (reason) {

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: //Unable to shake hands
		printf("Handshake failed\n");
		break;

	case LWS_CALLBACK_ESTABLISHED: //Handshake complete
		printf("Connection established\n");
		printf((char *) in);
		break;

	case LWS_CALLBACK_RECEIVE:
		printf("%d\n", (int) len);
		break;

	case LWS_CALLBACK_CLOSED:
		printf("Connection Closed");
		break;
	default:
		break;
	}
	return 0;
}

static struct libwebsocket_protocols protocols[] = { { "init", callback_init, 0, 10*1024*1024, 0 }, { NULL, NULL, 0 } };

void sighandler(int sig) {
	force_exit = 1;
}

int main(void) {
	signal(SIGINT, sighandler);

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));
	info.port = 8888;
	info.gid = -1;
	info.uid = -1;
	info.protocols = protocols;

	printf("starting server...\n");
	context = libwebsocket_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "libwebsocket init failed\n");
		return EXIT_FAILURE;
	}

	while (!force_exit) {
		libwebsocket_service(context, 500);
	}
	return EXIT_SUCCESS;
}

