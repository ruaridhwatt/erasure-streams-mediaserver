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

#define PORT_STR_LEN 6
#define PEER_ARRAY_INITIAL_SIZE 20

int myPort;
char myPortStr[PORT_STR_LEN];
int myId;

struct libwebsocket **peerArr;

int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);

int sendMyPort(struct libwebsocket *wsi);

int parseInitVars();

int getPeerList(struct libwebsocket *wsi);

int connectToPeer(char *peerStr, struct libwebsocket_context *ctx);

int addPeer(struct libwebsocket *wsi, struct libwebsocket **peerArr, size_t *index, size_t *peerArrSize);

void removePeer(struct libwebsocket *wsi, struct libwebsocket **peerArr, size_t *nrPeers);

#endif /* SRC_INTERN_H_ */
