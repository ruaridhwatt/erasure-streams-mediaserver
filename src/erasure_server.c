/*
 ============================================================================
 Name        : erasure_server.c
 ============================================================================
 Author      : dv11ann, dv12rwt
 Version     : 0.0
 ============================================================================
 */

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "file_utilities.h"
#include "upload.h"
#include "info.h"
#include "audio.h"
#include "video.h"
#include "intern.h"
#include "erasure_server.h"

enum Protocol {
	UPLOAD_PROTO = 0, INFO_PROTO = 1, AUDIO_PROTO = 2, VIDEO_PROTO = 3, INTERN_PROTO = 4
};

static struct libwebsocket_protocols protocols[] = { { "upload", callback_upload, sizeof(struct upload_user), RX_BUFFER_SIZE }, { "info",
		callback_info, sizeof(struct toSend), RX_BUFFER_SIZE }, { "audio", callback_audio, sizeof(struct toSend), RX_BUFFER_SIZE }, { "video",
		callback_video, sizeof(struct toSend), RX_BUFFER_SIZE }, { "intern", callback_intern, sizeof(peer), RX_BUFFER_SIZE }, { NULL, NULL, 0 } };

int main(int argc, char *argv[]) {
	int res, c, nsPort;
	char *nameServer, *streamToDist;
	char *envVars[NR_ENV_VARS];
	struct lws_context_creation_info info;
	struct libwebsocket_context *context;
	struct libwebsocket *nameServerWsi;

	signal(SIGINT, sighandler);

	envVars[0] = VIDEO_DIR_ENV_VAR;
	envVars[1] = BENTO4_ENV_VAR;
	envVars[2] = CRS_ENV_VAR;
	envVars[3] = SCRIPT_ENV_VAR;
	res = verifyEnvironmentSettings(envVars, NR_ENV_VARS);
	if (res < 0) {
		exit(1);
	}

	/* Parse command line arguments */
	nameServer = NULL;
	myPort = -1;
	nsPort = -1;
	while ((c = getopt(argc, argv, "s:t:p:")) != -1) {
		switch (c) {
		case 's':
			nameServer = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
			if (nameServer != NULL) {
				strcpy(nameServer, optarg);
			}
			break;
		case 't':
			res = str2int(optarg, &nsPort);
			break;
		case 'p':
			res = str2int(optarg, &myPort);
			break;
		case '?':
			print_usage(argv[0]);
			exit(1);
			break;
		default:
			print_usage(argv[0]);
			exit(1);
		}
		if (res < 0) {
			break;
		}
	}

	if (res < 0 || nameServer == NULL || nsPort == -1 || myPort < 1024) {
		print_usage(argv[0]);
		if (nameServer != NULL) {
			free(nameServer);
		}
		exit(1);
	}

	snprintf(myPortStr, MAX_PORT_LEN, "%d", myPort);

	res = pthread_mutex_init(&fmux, NULL);
	if (res != 0) {
		perror("pthread_mutex_init");
		free(nameServer);
		exit(1);
	}

	res = pthread_mutex_init(&lmux, NULL);
	if (res != 0) {
		perror("pthread_mutex_init");
		pthread_mutex_destroy(&fmux);
		free(nameServer);
		exit(1);
	}

	memset(&info, 0, sizeof(info));
	info.port = myPort;
	info.gid = -1;
	info.uid = -1;
	info.protocols = protocols;

	printf("starting server...\n");
	context = libwebsocket_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "libwebsocket init failed\n");
		free(nameServer);
		pthread_mutex_destroy(&fmux);
		pthread_mutex_destroy(&lmux);
		return EXIT_FAILURE;
	}

	toDist = llist_empty(free);

	nameServerWsi = libwebsocket_client_connect(context, nameServer, nsPort, 0, "/", nameServer, "origin", "intern", -1);
	if (nameServerWsi == NULL) {
		fprintf(stderr, "libwebsocket init failed\n");
		llist_free(toDist);
		free(nameServer);
		pthread_mutex_destroy(&fmux);
		pthread_mutex_destroy(&lmux);
		libwebsocket_context_destroy(context);
		return EXIT_FAILURE;
	}

	while (!force_exit) {
		libwebsocket_service(context, 500);
		pthread_mutex_lock(&lmux);
		if (!llist_isEmpty(toDist)) {
			streamToDist = llist_inspect(llist_first(toDist));
			res = distribute(streamToDist);
			if (res == 0) {
				llist_remove(llist_first(toDist), toDist);
			}
		}
		pthread_mutex_unlock(&lmux);
	}
	printf("stopping server...\n");
	llist_free(toDist);
	free(nameServer);
	pthread_mutex_destroy(&fmux);
	pthread_mutex_destroy(&lmux);
	libwebsocket_context_destroy(context);
	return EXIT_SUCCESS;
}

void sighandler(int sig) {
	force_exit = 1;
}

int verifyEnvironmentSettings(char **envVars, size_t nrVars) {
	int i, res;
	char *dest;

	res = 0;
	for (i = 0; i < nrVars; i++) {
		dest = getenv(envVars[i]);
		if (dest == NULL) {
			fprintf(stderr, "%s environment variable not set!\n", envVars[i]);
			res = -1;
			break;
		}
		if (dest[strlen(dest) - 1] != '/') {
			fprintf(stderr, "%s environment variable must end in /!\n", envVars[i]);
			res = -1;
			break;
		}
	}
	return res;
}

void print_usage(char *prog) {
	fprintf(stderr, "%s -s <nameServerURL> -t <nameServerPort> -p <listenPort>\n", prog);
}

