#!/bin/sh
if [ "$#" -ge 1 ] && [ "$#" -le 2 ]
then
   filename="$1"
   dirstring="dir"
   noMp4=${filename%.mp4}  
   moviefrag=$noMp4

   bentoFragmentString="Bento4/bin/mp4fragment"

   bentoEndString=$bentoFragmentString" "$filename" "$moviefrag
   echo ${bentoEndString}
   $bentoEndString

   bentoDashString="Bento4/bin/mp4dash --output-dir="  

   bentoEndString=$bentoDashString$moviefrag"dir "$moviefrag
   echo ${bentoEndString}
   $bentoEndString
   
else
   echo "Wrong number of arguments"
fi

