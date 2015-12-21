#ifndef UPLOAD_H_
#define UPLOAD_H_

#include <stdbool.h>
#include <libwebsockets.h>


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

typedef enum {
	UPL = 1, UNKNOWN = 0
} UPL_CMDS;



char *appendString(char *s1, char *s2);
void removeMp4(char *fileName);
int receiveFile(void *in, size_t len, struct upload_user *user);
void *initBentoFragmention(void *fName);
void uploadComplete(struct upload_user *thisUser);
UPL_CMDS getUPLCommand(char *in);
int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len);

#endif /* UPLOAD_H_ */
