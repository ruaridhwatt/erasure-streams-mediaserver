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

#define MAX_PORT_LEN 6
#define MAX_ID_LEN 6
#define MAX_HOST_SIZE 45
#define PEER_ARRAY_INITIAL_SIZE 20

typedef struct _peer {
	int id;
	char host[MAX_HOST_SIZE];
	int port;
	struct libwebsocket *wsi;
	char expecting[4];
} peer;

int myPort;
char myPortStr[MAX_PORT_LEN];
int myId;

int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);

int sendMyPort(struct libwebsocket *wsi);

int getInfo(struct libwebsocket *wsi);

int sendInfo(peer *me, struct libwebsocket *wsi);

peer *fillPeer(peer *p, int *res);

int setInitVars();

int getPeerList(struct libwebsocket *wsi);

peer *connectToPeer(char *peerStr, struct libwebsocket_context *ctx);

peer **addPeer(peer *p, peer **peerArr, size_t *index, size_t *peerArrSize, int *res);

peer **removePeer(peer *p, peer **peerArr, size_t *nrPeers);

int distribute(char *streamDir);

#endif /* SRC_INTERN_H_ */
