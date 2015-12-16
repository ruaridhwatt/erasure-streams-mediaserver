#include "stream_video_audio.h"

void setTotalFragments(struct per_session_data *psd) {
	long slicenr = 0;
	long remain = psd->filesize;
	while (remain) {
		long toCpy = remain > PAYLOAD-5 ? PAYLOAD-5 : remain;
		remain -= toCpy;
		slicenr++;
	}
	psd->total_packets = slicenr;
}

void initFileInfo(struct per_session_data *psd) {
	if( access( psd->filedir, F_OK ) != -1 ) {
		FILE *fileptr = fopen(psd->filedir, "rb");
		fseek(fileptr, 0, SEEK_END);
		long filelen = ftell(fileptr);
		rewind(fileptr);
		psd->filesize = filelen;
		setTotalFragments(psd);
		fprintf(stderr, "file read success %s\n", psd->filedir);
		fclose(fileptr);
	} else {
		fprintf(stderr, "fileread fail\n");
	    psd->nok = true;
	}
}

unsigned char *getFragment(struct per_session_data *psd) {
	fprintf(stderr, "filedir: %s\n", psd->filedir);
	FILE *fileptr = fopen(psd->filedir, "rb");
	long remain =psd->filesize;
	long slicenr = 0;
	rewind(fileptr);

	unsigned char *buffer = (unsigned char *)malloc((PAYLOAD)*sizeof(unsigned char));
	while (remain) {
		long toCpy = remain > PAYLOAD ? PAYLOAD : remain;
		remain -= toCpy;
		fread(buffer, 1, toCpy, fileptr);
		if(slicenr == psd->package_to_send) {
			psd->sendsize = toCpy;
			break;
		}
		slicenr++;
	}
	fprintf(stderr, "sendsize: %d\n", psd->sendsize);
	fclose(fileptr);
	return buffer;

}

void stream_audio_video(char* in, struct per_session_data *psd, struct libwebsocket_context *ctx, struct libwebsocket *wsi, bool isVideo) {
	char *fileName = (char*)in;

	if(startsWith(INI, fileName)) {
		fileName += strlen(INI) + 1;
		if(isDirectory(fileName)) {
			removeSubstring(fileName, ".mp4");
			if(isVideo) {
				psd->filedir = stringAppender(fileName, "/vinit");
			} else {
				psd->filedir = stringAppender(fileName, "/ainit");
			}
			psd->package_to_send = 0;
			psd->nok = false;
			fprintf(stderr, "filedir: %s\n", psd->filedir);
			initFileInfo(psd);
			libwebsocket_callback_on_writable(ctx, wsi);
		} else {
			psd->nok = true;
			libwebsocket_callback_on_writable(ctx, wsi);
		}
	} else if(startsWith(GET, fileName)) {
		fileName += strlen(GET) + 1;
		strtok(fileName, TAB);
		char *sliceNr =  strtok(NULL, TAB);

		if(isDirectory(fileName)) {
			char *dir;
			if(isVideo) {
				dir = setVideoDirectory(fileName);
			} else {
				dir = setAudioDirectory(fileName);
			}

			char *tempString1 = stringAppender(dir, "d");
			psd->filedir = stringAppender(tempString1, sliceNr);
			psd->package_to_send = 0;
			fprintf(stderr, "filedir: %s\n", psd->filedir);

			free(tempString1);
			free(dir);
			psd->nok = false;
			initFileInfo(psd);
			libwebsocket_callback_on_writable(ctx, wsi);
		} else {
			psd->nok = true;
			libwebsocket_callback_on_writable(ctx, wsi);
		}
	}
}

