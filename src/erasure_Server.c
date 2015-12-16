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

#include "erasure_Server.h"

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
//	putVideoInList("test.mp4");
//	putVideoInMap("test.mp4");
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
