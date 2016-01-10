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
#define MAX_HOST_SIZE 45

typedef struct _peer {
	int id;
	char host[MAX_HOST_SIZE];
	int port;
	struct libwebsocket *wsi;
	int sentInfo;
} peer;

int myPort;
char myPortStr[PORT_STR_LEN];
int myId;

int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);

int sendMyPort(struct libwebsocket *wsi);

int sendInfo(peer *me, struct libwebsocket *wsi);

peer *fillPeer(peer *p);

int parseInitVars();

int getPeerList(struct libwebsocket *wsi);

peer *connectToPeer(char *peerStr, struct libwebsocket_context *ctx);

peer **addPeer(peer *p, peer **peerArr, size_t *index, size_t *peerArrSize, int *res);

peer **removePeer(peer *p, peer **peerArr, size_t *nrPeers);

#endif /* SRC_INTERN_H_ */
