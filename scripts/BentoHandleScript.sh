#!/bin/bash
if [ "$#" -ge 3 ] && [ "$#" -le 4 ]
then
   filename=$1
   directory=$VIDEO_DIR"."${filename%.*}  

   res= cd $directory
   if [ ${#res} -gt 0 ]; then
      echo "Can not reach directory"
      exit -1
   fi

   bentoFragmentString=$BENTO4_HOME"bin/mp4fragment "$1" frag.mp4 --fragment-duration 86400000"
   if ! $bentoFragmentString; then
      printf '%s\n' 'Error running bento fragmentation' >&2
      exit -1
   fi

   if ! rm $1; then
      printf '%s\n' 'Error removing mp4 file' >&2
      exit -1
   fi 

   bentoDashString=$BENTO4_HOME"bin/mp4dash --mpd-name=stream.mpd --no-media frag.mp4 -o ./"
   if ! $bentoDashString; then
      printf '%s\n' 'Error running bento dash' >&2
      exit -1
   fi

   bentoSplitAudio=$BENTO4_HOME"bin/mp4split frag.mp4 --audio --media-segment adata --init-segment ainit.mp4"
   if ! $bentoSplitAudio; then
      printf '%s\n' 'Error splitting audio' >&2
      exit -1
   fi

   bentoSplitVideo=$BENTO4_HOME"bin/mp4split frag.mp4 --video --media-segment vdata --init-segment vinit.mp4"
   if ! $bentoSplitVideo; then
      printf '%s\n' 'Error splitting video' >&2
      exit -1
   fi

   if ! rm 'frag.mp4'; then
      printf '%s\n' 'Error removing mp4 file' >&2
      exit -1
   fi 

   crs_audstring=$CRS_HOME"bin/crs_erasure_codes -e -k"$2" -m"$3" adata aenc"
   if ! $crs_audstring; then
      printf '%s\n' 'Error erasure coding audio' >&2
      exit -1
   fi

   if ! rm 'adata'; then
      printf '%s\n' 'Error removing file adata' >&2
      exit -1
   fi 

   crs_vidstring=$CRS_HOME"bin/crs_erasure_codes -e -k"$2" -m"$3" vdata venc"
   if ! $crs_vidstring; then
      printf '%s\n' 'Error erasure coding video' >&2
      exit -1
   fi

   if ! rm 'vdata'; then
      printf '%s\n' 'Error removing file adata' >&2
      exit -1
   fi 
   
else
   echo "Wrong number of arguments"
   exit -1
fi
