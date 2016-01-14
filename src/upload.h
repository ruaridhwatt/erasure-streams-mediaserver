#ifndef UPLOAD_H_
#define UPLOAD_H_

#include <stdbool.h>
#include <libwebsockets.h>


struct upload_user {
	char *filename;
	FILE *f;
};

/**
 * Callback for the "upload" websockets protocol
 * @param ctx The context
 * @param wsi The client connection
 * @param reason The reason for the callback
 * @param user The allocated per user data
 * @param in The data received
 * @param len The length in bytes of the received data
 * @return 0 for success, otherwise -1
 */
int callback_upload(struct libwebsocket_context * ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len);

/**
 * Writes the string to the websocket
 * @param string The string to send
 * @param wsi The websocket to write to
 * @return 0 on success, otherwise -1
 */
int send_text(char *string, struct libwebsocket *wsi);

#endif /* UPLOAD_H_ */
