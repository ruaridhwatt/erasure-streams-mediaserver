/*
 * intern.c
 *
 *  Created on: 18 Dec 2015
 *      Author: dv12rwt
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "file_utilities.h"
#include "intern.h"

const int nrInternCommands = 4;
const char *internCommandStr[] = { "gin", "pin", "ini", "lst" };
enum InternCommand {

	GET_INFO = 0, PUT_INFO = 1, INIT = 2, PEER_LIST = 3,

	UNKNOWN = 4
};

enum Mode {
	NORM, DIST, REPAIR
};

enum Mode m = NORM;
peer **peerArr = NULL;
size_t peerArrSize = 0;
size_t nrPeers = 0;

/**
 * Converts an internal command string to the corresponding InternCommand enum
 * @param command The command string
 * @return The corresponding InfoCommand enum
 */
enum InternCommand getInternCommand(char *command) {
	int i;
	for (i = 0; i < nrInternCommands; i++) {
		if (strcmp(command, internCommandStr[i]) == 0) {
			break;
		}
	}
	return (enum InternCommand) i;
}

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
int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len) {

	static peer *me = NULL;

	enum InternCommand c;
	int res;
	char *peerStr;
	peer *p = (peer *) user;

	res = 0;
	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		fprintf(stderr, "New peer connected\n");
		res = getInfo(wsi);
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		fprintf(stderr, "Peer connection established\n");
		break;
	case LWS_CALLBACK_RECEIVE:
		/* no break */
	case LWS_CALLBACK_CLIENT_RECEIVE:
		if (lws_frame_is_binary(wsi)) {
			fprintf(stderr, "Unexpected binary data received\n");
		} else {

			fprintf(stderr, "%s\n", (char *) in);
			c = getInternCommand(strtok(in, "\t"));

			switch (c) {
			case INIT:
				res = setInitVars();
				if (res != 0) {
					fprintf(stderr, "Could not parse initialization variables from Name Server\n");
					force_exit = 1;
				} else {
					res = sendMyPort(wsi);
				}
				break;
			case PEER_LIST:
				while ((peerStr = strtok(NULL, "\t")) != NULL) {
					p = connectToPeer(peerStr, ctx);
					if (p == NULL) {
						res = -1;
						break;
					}
					if (p->wsi == NULL) {
						me = p;
					}
					peerArr = addPeer(p, peerArr, &nrPeers, &peerArrSize, &res);
					if (res != 0) {
						res = -1;
						break;
					}
				}
				if (res == 0) {
					fprintf(stdout, "Peer network established\n");
				} else {
					fprintf(stderr, "Could not establish peer network\n");
					force_exit = 1;
				}
				break;
			case GET_INFO:
				res = sendInfo(me, wsi);
				break;
			case PUT_INFO:
				p = fillPeer(p, wsi, &res);
				if (res == 0) {
					addPeer(p, peerArr, &nrPeers, &peerArrSize, &res);
				}
				break;
			default:
				break;
			}
		}
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		/* no break */
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		fprintf(stderr, "Writable callback for port %d\n", p->port);
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		/* no break */
	case LWS_CALLBACK_CLOSED:
		fprintf(stderr, "CLOSING\n");
		peerArr = removePeer(p, peerArr, &nrPeers);
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		fprintf(stderr, "Freeing Peer Array\n");
		freePeerArr();
		break;
	default:
		break;
	}

	return res;
}

/**
 * Writes the get info command to the wsi
 * @param wsi
 * @return 0 on success, otherwise -1
 */
int getInfo(struct libwebsocket *wsi) {
	char *buf;
	int res;
	size_t strSize;
	strSize = strlen(internCommandStr[GET_INFO]) + 1;
	buf = malloc(LWS_SEND_BUFFER_PRE_PADDING + (strSize * sizeof(char)) + LWS_SEND_BUFFER_POST_PADDING);
	if (buf == NULL) {
		return -1;
	}
	strcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], internCommandStr[GET_INFO]);
	res = libwebsocket_write(wsi, (unsigned char *) &buf[LWS_SEND_BUFFER_PRE_PADDING], strlen(&buf[LWS_SEND_BUFFER_PRE_PADDING]), LWS_WRITE_TEXT);
	free(buf);
	return res;
}

/**
 * Writes this peers info to the wsi
 * @param me This peer
 * @param wsi The wsi websocket to write to
 * @return 0 on success, otherwise -1
 */
int sendInfo(peer *me, struct libwebsocket *wsi) {
	char *buf, *dest;
	char idStr[MAX_ID_LEN];
	size_t strSize;
	int res;

	sprintf(idStr, "%d", myId);

	strSize = strlen(internCommandStr[PUT_INFO]) + 1;
	strSize += strlen(me->host) + 1;
	strSize += strlen(myPortStr) + 1;
	strSize += strlen(idStr) + 1;

	buf = malloc(LWS_SEND_BUFFER_PRE_PADDING + (strSize * sizeof(char)) + LWS_SEND_BUFFER_POST_PADDING);
	dest = strcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], internCommandStr[PUT_INFO]);
	dest = strcat(dest, "\t");
	dest = strcat(dest, me->host);
	dest = strcat(dest, ":");
	dest = strcat(dest, myPortStr);
	dest = strcat(dest, ":");
	dest = strcat(dest, idStr);

	res = libwebsocket_write(wsi, (unsigned char *) dest, strlen(dest), LWS_WRITE_TEXT);
	free(buf);
	return res;
}

/**
 * Fills the peer struct
 * @param p The peer to fill
 * @param wsi The peers wsi
 * @param res Where to provide the result (0 on success, otherwise -1)
 * @return The filled peer
 */
peer *fillPeer(peer *p, struct libwebsocket *wsi, int *res) {
	int port;

	strcpy(p->host, strtok(NULL, ":"));
	*res = str2int(strtok(NULL, ":"), &port);
	if (*res != 0) {
		return p;
	}
	p->port = port;
	*res = str2int(strtok(NULL, ":"), &(p->id));
	p->wsi = wsi;
	return p;
}

/**
 * Writes this peers port to the wsi
 * @param wsi Th websocket to write to
 * @return 0 on success, otherwise -1
 */
int sendMyPort(struct libwebsocket *wsi) {
	size_t strSize;
	char *buf, *dest;
	int res;

	strSize = strlen(MY_PORT_FOLLOWS_KW) + 1 + strlen(myPortStr) + 1;
	buf = (char *) malloc(LWS_SEND_BUFFER_PRE_PADDING + (strSize * sizeof(char)) + LWS_SEND_BUFFER_POST_PADDING);
	if (buf == NULL) {
		return -1;
	}
	dest = strcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], MY_PORT_FOLLOWS_KW);
	dest = strcat(dest, "\t");
	dest = strcat(dest, myPortStr);
	/* PRINT */
	fprintf(stderr, "%s\n", dest);
	res = libwebsocket_write(wsi, (unsigned char *) dest, strlen(dest), LWS_WRITE_TEXT);
	free(buf);
	return res;
}

/**
 * Stores the received initialization variables
 * @return 0 on success, otherwise -1
 */
int setInitVars() {
	char *idStr, *k, *m;
	int res;
	idStr = strtok(NULL, "\t");
	if (idStr == NULL) {
		return -1;
	}
	res = str2int(idStr, &myId);
	if (res != 0) {
		return -1;
	}
	k = strtok(NULL, "\t");
	if (k == NULL) {
		return -1;
	}
	strncpy(kStr, k, K_STR_LEN);
	kStr[K_STR_LEN - 1] = '\0';
	res = str2int(kStr, &kVal);
	if (res != 0) {
		return -1;
	}
	m = strtok(NULL, "\t");
	if (m == NULL) {
		return -1;
	}
	strncpy(mStr, m, M_STR_LEN);
	mStr[M_STR_LEN - 1] = '\0';
	res = str2int(mStr, &mVal);
	if (res != 0) {
		return -1;
	}
	if (kVal <= 0 || mVal <= 0 || mVal > kVal) {
		return -1;
	}
	return 0;
}

/**
 * Connects to the peer using the peer string received from the nameserver
 * @param peerStr The peers info (host:port:id)
 * @param ctx The websocket context
 * @return A filled peer struct or NULL if unsuccessful
 */
peer *connectToPeer(char *peerStr, struct libwebsocket_context *ctx) {
	char *peerHost, *peerPortStr, *peerIdStr;
	int res, peerPort, peerId;
	struct libwebsocket *wsi;
	peer *p;

	wsi = NULL;
	peerHost = peerStr;

	peerPortStr = index(peerStr, ':');
	*peerPortStr = '\0';
	if (strlen(peerHost) == 0 || strlen(peerHost) >= MAX_HOST_SIZE) {
		return NULL;
	}
	peerPortStr++;
	peerIdStr = index(peerPortStr, ':');
	*peerIdStr = '\0';
	if (strlen(peerPortStr) == 0) {
		return NULL;
	}
	peerIdStr++;

	res = str2int(peerPortStr, &peerPort);
	if (res != 0) {
		return NULL;
	}
	res = str2int(peerIdStr, &peerId);
	if (res != 0) {
		return NULL;
	}

	/* PRINT */
	fprintf(stderr, "%s\t%s\t%s\n", peerHost, peerPortStr, peerIdStr);

	p = (peer *) malloc(sizeof(peer));
	p->expecting[0] = '\0';
	p->id = peerId;
	strcpy(p->host, peerHost);
	p->port = peerPort;

	if (peerId != myId) {
		wsi = libwebsocket_client_connect_extended(ctx, peerHost, peerPort, 0, "/", peerHost, "origin", "intern", -1, p);
		if (wsi == NULL) {
			free(p);
			return NULL;
		}
	}
	p->wsi = wsi;
	return p;
}

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
peer **addPeer(peer *p, peer **peerArr, size_t *nrPeers, size_t *peerArrSize, int *res) {
	int i;
	peer **reallocedPeerArr;

	*res = 0;
	if (*nrPeers == *peerArrSize) {
		*peerArrSize += PEER_ARRAY_INITIAL_SIZE;
		reallocedPeerArr = realloc(peerArr, *peerArrSize * sizeof(peer *));
		if (reallocedPeerArr == NULL) {
			*res = -1;
			return peerArr;
		} else {
			peerArr = reallocedPeerArr;
		}
	}
	peerArr[*nrPeers] = p;
	(*nrPeers)++;

	/* PRINT */
	fprintf(stderr, "Added: ");
	fprintf(stderr, "[%d:%p", peerArr[0]->port, (void *) peerArr[0]->wsi);
	for (i = 1; i < *nrPeers; i++) {
		fprintf(stderr, ", %d:%p", peerArr[i]->port, (void *) peerArr[i]->wsi);
	}
	fprintf(stderr, "]\n");

	return peerArr;
}

/**
 * Removes a peer from the array of peers
 * @param p The peer to remove
 * @param peerArr The peer array
 * @param nrPeers A pointer to the current number of peers (this will be updated on removal)
 * @return the updated peer array
 */
peer **removePeer(peer *p, peer **peerArr, size_t *nrPeers) {
	int i;
	for (i = 0; i < *nrPeers; i++) {
		if (p->id == peerArr[i]->id) {
			break;
		}
	}
	if (i != *nrPeers) {
		memmove(&peerArr[i], &peerArr[i + 1], (*nrPeers - i - 1) * sizeof(peer *));
		(*nrPeers)--;
	} else {
		fprintf(stderr, "Nameserver closed!\n");
	}

	/* PRINT */
	fprintf(stderr, "removed: ");
	if (*nrPeers != 0) {
		fprintf(stderr, "[%d", peerArr[0]->port);
	}
	for (i = 1; i < *nrPeers; i++) {
		fprintf(stderr, ", %d", peerArr[i]->port);
	}
	fprintf(stderr, "]\n");

	return peerArr;
}

/**
 * TODO Update the peers with a queue of files to be sent and request a writable callback.
 * @param streamDir The stream directory
 * @return 0 on success, otherwise -1
 */
int distribute(char *streamDir) {
	if (m == NORM) {
		m = DIST;
		fprintf(stderr, "Distribute %s\n", streamDir);
		return 0;
	}
	return -1;
}

/**
 * Creates a redirect string to be sent to a client given the segment number. (switch-server\tws://host:port)
 * @param segNr
 * @return The redirect command
 */
char *getRedirect(int segNr) {
	peer *p;
	char portStr[MAX_PORT_LEN];
	char *buf, *dest;
	size_t strSize;

	p = getPeer(segNr, DATA);
	if (p->wsi == NULL) {
		return NULL;
	}
	sprintf(portStr, "%d", p->port);

	strSize = strlen(SWITCH_SERVER_KW) + 1;
	strSize += strlen(WS_PROTO_STR);
	strSize += strlen(p->host) + 1;
	strSize += strlen(portStr) + 1;

	buf = malloc(LWS_SEND_BUFFER_PRE_PADDING + (strSize * sizeof(char)) + LWS_SEND_BUFFER_POST_PADDING);
	dest = strcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], SWITCH_SERVER_KW);
	dest = strcat(dest, "\t");
	dest = strcat(dest, WS_PROTO_STR);
	dest = strcat(dest, p->host);
	dest = strcat(dest, ":");
	dest = strcat(dest, portStr);
	fprintf(stderr, "%s\n", dest);

	return buf;
}

/**
 * Gets the peer that should have the given segment type/number
 * @param segNr The segment number
 * @param type The segment type
 * @return The peer that should hold the data
 */
peer *getPeer(int segNr, enum SegType type) {
	int fileNr, filesPerPeer, index;

	fileNr = (type == DATA) ? (segNr - 1) : (segNr + kVal - 1);

	filesPerPeer = (int) round((kVal + mVal) / ((double) nrPeers));

	index = fileNr / filesPerPeer;

	if (index >= nrPeers) {
		index = nrPeers - 1;
	}
	fprintf(stderr, "SegNr %d\n", segNr);
	fprintf(stderr, "Redirecting to ms nr %d wsi address %p\n", index, (void *) peerArr[index]->wsi);
	return peerArr[index];
}

/**
 * frees the peer array
 */
void freePeerArr() {
	int i;
	if (peerArr != NULL) {
		for (i = 0; i < nrPeers; i++) {
			free(peerArr[i]);
		}
		free(peerArr);
	}
}

