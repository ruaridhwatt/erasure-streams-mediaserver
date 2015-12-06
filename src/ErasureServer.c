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
#include <libwebsockets.h>

#define PAYLOAD 1024*1024
#define OK "OK"
#define SWITCH_SERVER "switch-server"
#define START_STREAM "start-stream"

static volatile int force_exit = 0;
static struct libwebsocket_context *context;

struct upload_user {
   bool grantedUpload;
   bool terminatorReceived;
};

struct per_session_data {
	long total_packets;
	long package_to_send;
	int readFile;
	long filesize;
	int sendsize;
	unsigned char *buff;
	char *filename;
	int shouldSendFile;
	int endPacketsSent;
	int isSendingFile;
};

/*
 * The video name is hardcoded for now
 *
 */
void receiveFile(void *in, size_t len, struct upload_user *user) {
   if (len == 1 && *((unsigned char *) in) == 0) {
      if (user->terminatorReceived == true) {
         /* Second terminator received */
         printf("Upload done!\n");
         user->grantedUpload = false;
         user->terminatorReceived = false;
         /* TODO start distribution */
      }
      user->terminatorReceived = true;
   } else {
      printf("%d\n", (int)len);
      FILE *received_file = fopen("movie5.mp4", "a+b");
      if (received_file == NULL) {
         fprintf(stderr, "Failed to open file movie5.mp4\n");
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
      libwebsocket_write(wsi, &buff[LWS_SEND_BUFFER_PRE_PADDING], length, write_mode);
      free(buff);
      return EXIT_SUCCESS;
   }
   return EXIT_FAILURE;
}

void readFileBytes(struct per_session_data *psd) {
	FILE *fileptr = fopen(psd->filename, "rb");
	fseek(fileptr, 0, SEEK_END);
	long filelen = ftell(fileptr);
	rewind(fileptr);

	unsigned char *buffer = (unsigned char *)malloc((filelen)*sizeof(unsigned char));
	fread(buffer, 1, filelen, fileptr);
	fclose(fileptr);

	psd->buff = buffer;
	psd->filesize = filelen;
}

void getTotalFragments(struct per_session_data *psd) {
	long slicenr = 0;
	long remain = psd->filesize;
	while (remain) {
		long toCpy = remain > PAYLOAD-5 ? PAYLOAD-5 : remain;
		remain -= toCpy;
		slicenr++;
	}
	psd->total_packets = slicenr;
}

unsigned char *getFragment(struct per_session_data *psd) {

	unsigned char *buf = (unsigned char *)malloc((PAYLOAD - 4)*sizeof(unsigned char));

	long slicenr = 0;
	long remain =psd->filesize;
	unsigned char* buffer = psd->buff;
	while (remain)
	{
		long toCpy = remain > PAYLOAD-5 ? PAYLOAD-5 : remain;
		memcpy(buf, buffer, toCpy);
		buffer += toCpy;
		remain -= toCpy;
		if(slicenr == psd->package_to_send) {
			psd->sendsize = toCpy;
			break;
		}
		slicenr++;
	}
	return buf;
}

bool startsWith(const char *pre, const char *str) {
	return strncmp(pre, str, strlen(pre)) == 0;
}

static int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {

	struct upload_user * thisUser = (struct upload_user *) user;

	switch (reason) {

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: //Unable to shake hands
		printf("Handshake failed\n");
		break;

	case LWS_CALLBACK_ESTABLISHED: //Handshake complete, create user.
		printf("Connection established\n");
		thisUser->grantedUpload = false;
		thisUser->terminatorReceived = false;
      break;

   case LWS_CALLBACK_RECEIVE: //stores the received file as "movie.mp4" in src folder.
      if (lws_frame_is_binary(wsi) && thisUser->grantedUpload) {
         receiveFile(in, len, thisUser);
      } else {
         printf((char*) in);
         thisUser->grantedUpload = true;
         send_message(OK, strlen(OK), LWS_WRITE_TEXT, wsi);
      }
      break;

   case LWS_CALLBACK_CLOSED:
      printf("Connection Closed");
      break;
   default:
      break;
   }
   return 0;
}

static int callback_stream(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len) {

	struct per_session_data *psd = user;

	switch (reason) {

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		break;
	case LWS_CALLBACK_ESTABLISHED:
		printf("Client connection established\n");
		break;
	case LWS_CALLBACK_RECEIVE:
		if (!lws_frame_is_binary(wsi)) {
			char *inString = (char*)in;

			if(startsWith(START_STREAM, inString)) {
				inString += strlen(START_STREAM) + 1;
				printf("request received, streaming %s\n", inString);
				psd->readFile = 1;
				psd->filename = inString;
				psd->isSendingFile = 1;
				libwebsocket_callback_on_writable(ctx, wsi);
			}
		}
		break;
	case LWS_CALLBACK_CLOSED:
		printf("Connection Closed");
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:

		if(psd->isSendingFile) {
			if(psd->readFile) {
				psd->readFile = 0;
				readFileBytes(psd);
				getTotalFragments(psd);
				psd->package_to_send = 0;
				psd->endPacketsSent = 0;
			}

			if(psd->package_to_send < psd->total_packets){
				unsigned char* buf = getFragment(psd);
				send_message(buf, psd->sendsize, LWS_WRITE_BINARY, wsi);
				psd->package_to_send = psd->package_to_send + 1;
				libwebsocket_callback_on_writable(ctx, wsi);
			} else if (psd->endPacketsSent < 2) {
				char *buff = "0";
				unsigned char *buf = (unsigned char*)buff;
				send_message(buf, 1, LWS_WRITE_BINARY, wsi);
				psd->endPacketsSent = psd->endPacketsSent + 1;
				libwebsocket_callback_on_writable(ctx, wsi);
			} else {
				psd->isSendingFile = 0;
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

static struct libwebsocket_protocols protocols[] = {
      { "upload", callback_upload, sizeof(struct upload_user), PAYLOAD, 0 },
	  {"stream", callback_stream, sizeof(struct per_session_data) , PAYLOAD}, { NULL, NULL, 0 } };

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

