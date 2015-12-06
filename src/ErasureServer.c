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

#define PAYLOAD 10*1024*1024
#define OK "OK"
#define SWITCH_SERVER "switch-server"

static volatile int force_exit = 0;
static struct libwebsocket_context *context;

struct upload_user {
   bool grantedUpload;
   bool terminatorReceived;
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

/*
 * Slices up a file into pieces of size determined by PAYLOAD. Sends each piece to a given wsi with function send_message. Must be called by a writeable_callback
 *  as defined by libwebsockets.readme.
 *
 */
void readFileBytes(struct libwebsocket *wsi, char *filename) {
   FILE *fileptr;
   unsigned char *buffer;
   long filelen;

   fileptr = fopen(filename, "rb");
   fseek(fileptr, 0, SEEK_END);
   filelen = ftell(fileptr);
   rewind(fileptr);

   buffer = (unsigned char *) malloc((filelen + 1) * sizeof(unsigned char));
   fread(buffer, 1, filelen, fileptr);

   unsigned char *buf = (unsigned char *) malloc((PAYLOAD - 4) * sizeof(unsigned char));

   if (filelen < PAYLOAD - 5) {
      send_message(buf, filelen, LWS_WRITE_BINARY, wsi);
   } else {

      int remain = filelen;
      int flag = LWS_WRITE_BINARY | LWS_WRITE_NO_FIN;
      while (remain) {
         long toCpy = remain > PAYLOAD - 5 ? PAYLOAD - 5 : remain;
         memcpy(buf, buffer, toCpy);
         buffer += toCpy;
         remain -= toCpy;

         send_message(buf, toCpy, flag, wsi);
         if (remain < 1) {
            flag = LWS_WRITE_CONTINUATION;
         } else {
            flag = LWS_WRITE_CONTINUATION | LWS_WRITE_NO_FIN;
         }
      }
   }
   fclose(fileptr);
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

static int callback_DL_slice(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
      enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {

   switch (reason) {

   case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
      break;
   case LWS_CALLBACK_ESTABLISHED:
      printf("Client connection established\n");
      break;
   case LWS_CALLBACK_RECEIVE:
      printf("message received, sending movie.mp4\n");
      libwebsocket_callback_on_writable(ctx, wsi);
      break;
   case LWS_CALLBACK_CLOSED:
      break;
   case LWS_CALLBACK_HTTP:
      break;
   case LWS_CALLBACK_SERVER_WRITEABLE:
      //For now, hardcode the videon you want to send below.
      readFileBytes(wsi, "movie.mp4");
      printf("Sending video data from file\n");
      break;
   default:
      break;
   }
   return 0;
}

static struct libwebsocket_protocols protocols[] = {
      { "upload", callback_upload, sizeof(struct upload_user), PAYLOAD, 0 },
      { "DL_Slice", callback_DL_slice, 0, PAYLOAD }, { NULL, NULL, 0 } };

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

