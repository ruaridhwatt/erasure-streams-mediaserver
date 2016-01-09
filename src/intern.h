/*
 * intern.h
 *
 *  Created on: 18 Dec 2015
 *      Author: dv12rwt
 */

#ifndef SRC_INTERN_H_
#define SRC_INTERN_H_

#include <libwebsockets.h>

#define MY_PORT_FOLLOWS_KW "prt"
#define GET_PEER_LIST_KW "lst"

#define PORT_STR_LEN 6
#define PEER_ARRAY_INITIAL_SIZE 20

int myPort;
char myPortStr[PORT_STR_LEN];
int myId;

struct peer {
	struct libwebsocket *wsi;
};

int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);

int sendMyPort(struct libwebsocket *wsi);

int parseInitVars();

int getPeerList(struct libwebsocket *wsi);

struct peer *getPeer(char *peerStr, struct libwebsocket_context *ctx);

int addPeer(struct peer *p, struct peer **peerArr, size_t *index, size_t *peerArrSize);

#endif /* SRC_INTERN_H_ */
