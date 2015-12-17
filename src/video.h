/*
 * video.h
 *
 *  Created on: 17 Dec 2015
 *      Author: dv12rwt
 */

#ifndef VIDEO_H_
#define VIDEO_H_

#include <libwebsockets.h>

int callback_video(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len);

#endif /* VIDEO_H_ */
