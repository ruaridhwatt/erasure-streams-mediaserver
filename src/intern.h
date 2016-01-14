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
#define SWITCH_SERVER_KW "switch-server"
#define WS_PROTO_STR "ws://"

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

/**
 * Callback for the "intern" websockets protocol
 * @param ctx The context
 * @param wsi The client connection
 * @param reason The reason for the callback
 * @param user The allocated per user data
 * @param in The data received
 * @param len The length in bytes of the received data
 * @return 0 for success, otherwise -1
 */
int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);

/**
 * Writes this peers port to the wsi
 * @param wsi Th websocket to write to
 * @return 0 on success, otherwise -1
 */
int sendMyPort(struct libwebsocket *wsi);

/**
 * Writes the get info command to the wsi
 * @param wsi
 * @return 0 on success, otherwise -1
 */
int getInfo(struct libwebsocket *wsi);

/**
 * Writes this peers info to the wsi
 * @param me This peer
 * @param wsi The wsi websocket to write to
 * @return 0 on success, otherwise -1
 */
int sendInfo(peer *me, struct libwebsocket *wsi);

/**
 * Fills the peer struct
 * @param p The peer to fill
 * @param wsi The peers wsi
 * @param res Where to provide the result (0 on success, otherwise -1)
 * @return The filled peer
 */
peer *fillPeer(peer *p, struct libwebsocket *wsi, int *res);

/**
 * Stores the received initialization variables
 * @return 0 on success, otherwise -1
 */
int setInitVars();

/**
 * Connects to the peer using the peer string received from the nameserver
 * @param peerStr The peers info (host:port:id)
 * @param ctx The websocket context
 * @return A filled peer struct or NULL if unsuccessful
 */
peer *connectToPeer(char *peerStr, struct libwebsocket_context *ctx);

/**
 * Adds a peer to the array of peers
 * @param p The peer to add
 * @param peerArr The peer array
 * @param nrPeers A pointer to the number of peers, this value will be updated
 * @param peerArrSize A pointer to the current size of the peer array (will be >= nrPeers). May be updated if the peer
 * array is reallocated
 * @param res 0 on success, otherwise -1
 * @return The updated peer array
 */
peer **addPeer(peer *p, peer **peerArr, size_t *index, size_t *peerArrSize, int *res);

/**
 * Removes a peer from the array of peers
 * @param p The peer to remove
 * @param peerArr The peer array
 * @param nrPeers A pointer to the current number of peers (this will be updated on removal)
 * @return the updated peer array
 */
peer **removePeer(peer *p, peer **peerArr, size_t *nrPeers);

/**
 * TODO Update the peers with a queue of files to be sent and request a writable callback.
 * @param streamDir The stream directory
 * @return 0 on success, otherwise -1
 */
int distribute(char *streamDir);

/**
 * Creates a redirect string to be sent to a client given the segment number. (switch-server\tws://host:port)
 * @param segNr
 * @return The redirect command
 */
char *getRedirect(int segNr);

/**
 * Gets the peer that should have the given segment type/number
 * @param segNr The segment number
 * @param type The segment type
 * @return The peer that should hold the data
 */
peer *getPeer(int segNr, enum SegType type);

/**
 * frees the peer array
 */
void freePeerArr();

#endif /* SRC_INTERN_H_ */
