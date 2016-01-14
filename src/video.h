/*
 * video.h
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */

#ifndef VIDEO_H_
#define VIDEO_H_

#include <libwebsockets.h>

/**
 * Callback for the "video" websockets protocol
 * @param ctx The context
 * @param wsi The client connection
 * @param reason The reason for the callback
 * @param user The allocated per user data
 * @param in The data received
 * @param len The length in bytes of the received data
 * @return 0 for success, otherwise -1 (sends a close signal to the client)
 */
int callback_video(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len);

#endif /* VIDEO_H_ */
