#!/bin/bash

if [ "$#" -ne 3 ]
then
	printf '%s\n\t%s\n' 'Usage:' 'fragcodeMp4.sh <filepath> <nrDataFiles> <nrCodingFiles>' >&1
	exit 1
fi

filename=´basename $1´
directory=´dirname $1´  

if ! cd $directory; then
	exit 1
fi

bentoFragmentString=$BENTO4_HOME"bin/mp4fragment "filename" frag.mp4 --fragment-duration 86400000"
if ! $bentoFragmentString; then
	exit 1
fi

if ! rm $1; then
	exit 1
fi 

bentoDashString=$BENTO4_HOME"bin/mp4dash --mpd-name=stream.mpd --no-media frag.mp4 -o ./"
if ! $bentoDashString; then
	exit 1
fi

bentoSplitAudio=$BENTO4_HOME"bin/mp4split frag.mp4 --audio --media-segment adata --init-segment ainit.mp4"
if ! $bentoSplitAudio; then
	exit 1
fi

bentoSplitVideo=$BENTO4_HOME"bin/mp4split frag.mp4 --video --media-segment vdata --init-segment vinit.mp4"
if ! $bentoSplitVideo; then
	exit 1
fi

if ! rm 'frag.mp4'; then
	exit 1
fi 

crs_audstring=$CRS_HOME"bin/crs_erasure_codes -e -k"$2" -m"$3" adata aenc"
if ! $crs_audstring; then
	exit 1
fi

if ! rm 'adata'; then
	exit 1
fi 

crs_vidstring=$CRS_HOME"bin/crs_erasure_codes -e -k"$2" -m"$3" vdata venc"
if ! $crs_vidstring; then
	exit 1
fi

if ! rm 'vdata'; then
	exit 1
fi 
exit 0
