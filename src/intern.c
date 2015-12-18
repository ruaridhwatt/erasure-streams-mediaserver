/*
 * intern.c
 *
 *  Created on: 18 Dec 2015
 *      Author: dv12rwt
 */

#include <string.h>
#include "intern.h"

const int nrInternCommands = 14;
const char *internCommandStr[] = { "gsp", "gmp", "gin", "gda", "gco", "psp", "pmp", "pin", "pda", "pco", "pre", "pro", "acr", "acd" };
enum InternCommand {

	/* REQUESTS */
	GET_SPEC = 0, GET_MPD = 1, GET_INI = 2, GET_DATA = 3, GET_CODING = 4,

	/* PREPARES */
	PUT_SPEC = 5, PUT_MPD = 6, PUT_INI = 7, PUT_DATA = 8, PUT_CODING = 9,

	/* PAXOS */
	PREPARE = 10, PROMISE = 11, ACCEPT_REQUEST = 12, ACCEPTED = 13,

	UNKNOWN = 14
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
int callback_intern(struct libwebsocket_context *ctx, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in,
		size_t len) {
	int res;

	res = 0;
	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:

		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED:

		break;
	case LWS_CALLBACK_RECEIVE:

		break;
	case LWS_CALLBACK_CLIENT_RECEIVE:

		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:

		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE:

		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:

		break;
	case LWS_CALLBACK_CLOSED:

		break;
	default:
		break;
	}

	return res;
}

