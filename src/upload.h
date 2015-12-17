#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <unistd.h>

#define START_UPLOAD "upl"
#define BENTOSCRIPT "./scripts/BentoHandleScript.sh "

struct upload_user {
   bool grantedUpload;
   bool terminatorReceived;
   bool uploadComplete;
   char *filename;
   char *dir;
   char *dotDir;
   char *mp4Dir;
};

typedef enum {UPL = 1, UNKNOWN = 0} UPL_CMDS;

char *k;
char *m;
pthread_mutex_t mutex;

char *appendString(char *s1, char *s2);
void removeMp4(char *fileName);
void receiveFile(void *in, size_t len, struct upload_user *user);
void *initBentoFragmention(void *fName);
void uploadComplete(struct libwebsocket_context * ctx, struct upload_user *thisUser);
UPL_CMDS getUPLCommand(char *in) ;
int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);


