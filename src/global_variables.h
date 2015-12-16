#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <unistd.h>
#include <getopt.h>

#include "llist.h"
#include "hashmap.h"

#define INI "ini"
#define MPD "mpd"
#define GET "get"
#define TAB "\t"

#define PAYLOAD 10*1024
#define UPLOAD 1024*1024

#define START_UPLOAD "upl"
#define LIST_STREAMS "lst"

#define CMD_SIZE 3
#define BENTOSCRIPT "scripts/BentoHandleScript.sh "
#define FILEHANDLESCRIPT "./FileHandleScript.sh "

#define OK "OK"
#define NOK "NOK"
#define SWITCH_SERVER "switch-server"

#define EOA "EOA"
#define EOV "EOV"
#define EOS "EOS"


