#ifndef UPLOAD_H_
#define UPLOAD_H_

#include <stdbool.h>
#include <libwebsockets.h>

#define ACK "OK"
#define NACK "NOK"

struct upload_user {
	char *filename;
	FILE *f;
};

int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len);
int send(char *string, struct libwebsocket *wsi);

#endif /* UPLOAD_H_ */
