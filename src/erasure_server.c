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
#include "erasure_server.h"

static struct libwebsocket_protocols protocols[] = { { "upload", callback_upload, sizeof(struct upload_user), 0 }, { "info", callback_info,
		sizeof(struct toSend), 0 }, { "audio", callback_audio, sizeof(struct toSend), 0 }, { "video", callback_video, sizeof(struct toSend), 0 }, {
NULL, NULL, 0 } };

static volatile int force_exit = 0;

int main(int argc, char *argv[]) {
	int port, res, c, k, m;
	char *envVars[NR_ENV_VARS];
	struct lws_context_creation_info info;
	struct libwebsocket_context *context;

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
	port = -1;
	k = -1;
	m = -1;
	while ((c = getopt(argc, argv, "k:m:p:")) != -1) {
		switch (c) {
		case 'k':
			res = str2int(optarg, &k);
			break;
		case 'm':
			res = str2int(optarg, &m);
			break;
		case 'p':
			res = str2int(optarg, &port);
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

	if (res < 0 || port < 1024 || k <= 0 || m <= 0 || m > k) {
		print_usage(argv[0]);
		exit(1);
	}

	snprintf(kStr, K_STR_LEN, "%d", k);
	snprintf(mStr, M_STR_LEN, "%d", m);
	res = pthread_mutex_init(&mux, NULL);
	if (res != 0) {
		perror("pthread_mutex_init");
		exit(1);
	}

	/* Create websockets server */

	memset(&info, 0, sizeof(info));
	info.port = port;
	info.gid = -1;
	info.uid = -1;
	info.protocols = protocols;

	printf("starting server...\n");
	context = libwebsocket_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "libwebsocket init failed\n");
		pthread_mutex_destroy(&mux);
		return EXIT_FAILURE;
	}

	while (!force_exit) {
		libwebsocket_service(context, 500);
	}
	printf("stopping server...\n");
	pthread_mutex_destroy(&mux);
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
	fprintf(stderr, "%s -k <nrDataFiles> -m <nrCodingFiles> -p <port>\n", prog);
}

int str2int(char *str, int *i) {
	long l;
	char *pEnd;
	if (str == NULL) {
		return -1;
	}
	errno = 0;
	l = strtol(str, &pEnd, 10);
	if (pEnd == str || *pEnd != '\0' || errno == ERANGE) {
		return -1;
	}
	if (l > INT_MAX || l < INT_MIN) {
		errno = ERANGE;
		return -1;
	}
	*i = (int) l;
	return 0;
}

