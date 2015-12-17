/*
 * audio.h
 *
 *  Created on: Dec 17, 2015
 *      Author: dv12rwt
 */

#ifndef SRC_AUDIO_H_
#define SRC_AUDIO_H_

#include <libwebsockets.h>

int callback_audio(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason,
		void *user, void *in, size_t len);

#endif /* SRC_AUDIO_H_ */
