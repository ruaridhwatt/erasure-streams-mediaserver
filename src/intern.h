/*
 * intern.h
 *
 *  Created on: 18 Dec 2015
 *      Author: dv12rwt
 */

#ifndef SRC_INTERN_H_
#define SRC_INTERN_H_

#include <libwebsockets.h>
#include "hashmap.h"

#define MY_PORT_FOLLOWS_KW "prt"
#define GET_PEER_LIST_KW "lst"

#define PORT_STR_LEN 6
#define PEERMAP_INITIAL_SIZE 100

int myPort;
char myPortStr[PORT_STR_LEN];
int myId;

struct peer {
	int id;
	struct libwebsocket *wsi;
};

int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);

int parseInitVars();

struct peer *getPeer(char *peerStr, struct libwebsocket_context *ctx);

entry *createPeerEntry(struct peer *p);

unsigned long intHash(void *derefInt);

#endif /* SRC_INTERN_H_ */
