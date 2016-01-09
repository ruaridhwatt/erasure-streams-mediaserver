/*
 * intern.c
 *
 *  Created on: 18 Dec 2015
 *      Author: dv12rwt
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "file_utilities.h"
#include "intern.h"

const int nrInternCommands = 15;
const char *internCommandStr[] = { "gsp", "gmp", "gin", "gda", "gco", "psp", "pmp", "pin", "pda", "pco", "pre", "pro",
		"acr", "acd", "ini", "lst" };
enum InternCommand {

	/* REQUESTS */
	GET_SPEC = 0, GET_MPD = 1, GET_INI = 2, GET_DATA = 3, GET_CODING = 4,

	/* PREPARES */
	PUT_SPEC = 5, PUT_MPD = 6, PUT_INI = 7, PUT_DATA = 8, PUT_CODING = 9,

	/* PAXOS */
	PREPARE = 10, PROMISE = 11, ACCEPT_REQUEST = 12, ACCEPTED = 13,

	/* NAMESERVER */
	INIT_VARS = 14, PEER_LIST = 15,

	UNKNOWN = 16
};

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
int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {

	static struct libwebsocket *nameServerWsi = NULL;
	static hashmap *peerMap = NULL;

	struct peer *peer;
	enum InternCommand c;
	int res;
	char *buf, *peerStr;
	size_t sendLen;

	res = 0;
	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		/* PRINT */
		fprintf(stderr, "client connected");
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		if (nameServerWsi == NULL) {
			/* Name server connection established */
			nameServerWsi = wsi;
			sendLen = strlen(MY_PORT_FOLLOWS_KW) + 1 + strlen(myPortStr);
			buf = (char *) malloc(
			LWS_SEND_BUFFER_PRE_PADDING + ((sendLen + 1) * sizeof(char)) + LWS_SEND_BUFFER_POST_PADDING);
			if (buf == NULL) {
				res = -1;
			} else {
				strcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], MY_PORT_FOLLOWS_KW);
				strcat(&buf[LWS_SEND_BUFFER_PRE_PADDING], "\t");
				strcat(&buf[LWS_SEND_BUFFER_PRE_PADDING], myPortStr);
				/* PRINT */
				fprintf(stderr, "%s\n", &buf[LWS_SEND_BUFFER_PRE_PADDING]);
				res = libwebsocket_write(wsi, (unsigned char *) &buf[LWS_SEND_BUFFER_PRE_PADDING], sendLen,
						LWS_WRITE_TEXT);
				free(buf);
			}
		} else {
			/* Peer connection established */
			/* Add peer to list */
		}
		break;
	case LWS_CALLBACK_RECEIVE:

		break;
	case LWS_CALLBACK_CLIENT_RECEIVE:
		fprintf(stderr, "%s\n", (char *) in);
		c = getInternCommand(strtok(in, "\t"));

		switch (c) {
		case INIT_VARS:
			res = parseInitVars();
			if (res != 0) {
				fprintf(stderr, "Could not parse initialization variables (id, k, m) from Name Server\n");
			} else {
				buf = (char *) malloc(
						LWS_SEND_BUFFER_PRE_PADDING + ((strlen(GET_PEER_LIST_KW) + 1) * sizeof(char))
								+ LWS_SEND_BUFFER_POST_PADDING);
				if (buf == NULL) {
					res = -1;
				} else {
					strcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], GET_PEER_LIST_KW);
					/* PRINT */
					fprintf(stderr, "%s\n", &buf[LWS_SEND_BUFFER_PRE_PADDING]);
					res = libwebsocket_write(wsi, (unsigned char *) &buf[LWS_SEND_BUFFER_PRE_PADDING],
							strlen(GET_PEER_LIST_KW), LWS_WRITE_TEXT);
					free(buf);
				}
			}
			break;
		case PEER_LIST:
			while ((peerStr = strtok(NULL, "\t")) != NULL) {
				peer = getPeer(peerStr, ctx);
				if (peer == NULL) {
					fprintf(stderr, "Could not connect to peer at: %s", peerStr);
				} else {
					if (peerMap == NULL) {
						peerMap = hashmap_empty(PEERMAP_INITIAL_SIZE, intHash, free);
					}
					hashmap_put(createPeerEntry(peer), peerMap);
				}
			}
			break;
		default:
			break;
		}

		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:

		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE:

		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:

		break;
	case LWS_CALLBACK_CLOSED:

		break;
	case LWS_CALLBACK_PROTOCOL_DESTROY:
		if (peerMap != NULL) {
			hashmap_free(peerMap);
		}
		break;
	default:
		break;
	}

	return res;
}

int parseInitVars() {
	char *idStr, *k, *m;
	int res, kVal, mVal;
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

struct peer *getPeer(char *peerStr, struct libwebsocket_context *ctx) {
	char *peerHost, *peerPortStr, *peerIdStr;
	int res, peerPort, peerId;
	struct peer *peer;

	peerHost = peerStr;

	peerPortStr = index(peerStr, ':');
	*peerPortStr = '\0';
	if (strlen(peerHost) == 0) {
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
	peer = (struct peer *) malloc(sizeof(struct peer));
	peer->id = peerId;
	peer->wsi = libwebsocket_client_connect_extended(ctx, peerHost, peerPort, 0, "/", peerHost, "origin", "intern", -1,
			peer);
	if (peer->wsi == NULL) {
		free(peer);
		return NULL;
	}
	return peer;
}

unsigned long intHash(void *derefInt) {
	int i;
	unsigned long l;
	i = *(int *) derefInt;
	l = i;
	return l;
}

entry *createPeerEntry(struct peer *p) {
	entry *en;
	en = (entry *) malloc(sizeof(entry));
	en->key = &(p->id);
	en->key_size = sizeof(int);
	en->value = p;
	return en;
}

