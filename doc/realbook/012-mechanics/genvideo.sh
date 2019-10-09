#!/bin/sh
# generate a video out of png frames
if [ -z "$1" ] || [ -z "$2" ]; then
  echo usage: $0 basename fps
  echo finds files named basename-0000.png and builds a video named basename.mp4
  echo with the prescribed number of frames per seconds
  exit
fi

#avconv -y -f image2 -framerate $2 -i $1-%04d.png -vcodec libvpx $1.webm -v quiet
avconv -y -f image2 -framerate $2 -i $1-%04d.png $1.mp4 -v quiet
