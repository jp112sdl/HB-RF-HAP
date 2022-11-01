#!/bin/sh
if [ -z $2 ]; then
  echo "Parameter fehlt"
  exit 0
fi
if [ -z $1 ]; then
  echo "Parameter fehlt"
  exit 0
fi

fakesgtin=`echo -n "$1" | xxd -p`
fakesgtin_short=${fakesgtin:0:46}
origsgtin=`echo -n "$2" | xxd -p`
origsgtin_short=${origsgtin:0:46}
xxd -p -c180 $1.ap | sed 's/'"$fakesgtin_short"'/'"$origsgtin_short"'/g'| xxd -p -r > $2.ap

rm $1.ap
mv $1.apkx $2.apkx
mv $1.bbkx $2.bbkx

for f in ./*.dev; do
  echo "Processing $f file...";
  mv $f $f.bak
  xxd -p -c10000000 $f.bak | sed 's/'"$fakesgtin_short"'/'"$origsgtin_short"'/g'| xxd -p -r > $f
done