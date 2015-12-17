#!/bin/sh
if [ "$#" -ge 3 ] && [ "$#" -le 4 ]
then
   filename="$1"
   noMp4=${filename%.mp4}
   fragname=$noMp4"f.mp4"  

   directory="cd "$VIDEO_DIR"."$noMp4
   $directory   
   echo ${directory}
   
   bentoFragmentString=$BENTO4_HOME"bin/mp4fragment "$1" "$fragname" --fragment-duration 86400000"
   
   $bentoFragmentString   

   rmfile="rm "$1
   $rmfile 

   bentoDash=$BENTO4_HOME"bin/mp4dash --mpd-name=mpd --no-media "$fragname" -o ./"
   $bentoDash

   bentoSplitAudio=$BENTO4_HOME"bin/mp4split "$fragname" --audio --media-segment adata --init-segment ainit.mp4"
   $bentoSplitAudio

   bentoSplitVideo=$BENTO4_HOME"bin/mp4split "$fragname" --video --media-segment vdata --init-segment vinit.mp4"
   $bentoSplitVideo

   rmfile="rm "$fragname
   $rmfile 

   crs_audstring=$CRS_HOME"bin/crs_erasure_codes -e -k"$2" -m"$3" adata aenc"
   $crs_audstring

   rmdata="rm adata"
   $rmdata

   crs_vidstring=$CRS_HOME"bin/crs_erasure_codes -e -k"$2" -m"$3" vdata venc"
   $crs_vidstring

   rmdata="rm vdata"
   $rmdata

   
else
   echo "Wrong number of arguments"
fi
