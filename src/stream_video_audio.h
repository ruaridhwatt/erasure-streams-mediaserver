#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <unistd.h>

#include "string_functions.h"
#include "global_variables.h"
//#include "upload.h"

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

void setTotalFragments(struct per_session_data *psd);
void initFileInfo(struct per_session_data *psd);
unsigned char *getFragment(struct per_session_data *psd);
void stream_audio_video(char* in, struct per_session_data *psd, struct libwebsocket_context *ctx, struct libwebsocket *wsi, bool isVideo);
