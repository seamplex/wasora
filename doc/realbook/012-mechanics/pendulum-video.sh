besssugo pendulum-video.was newton 48
avconv -y -v quiet -f image2 -framerate 48 -i newton-%04d.png -r 48 newton.mp4
avconv -y -v quiet -f image2 -framerate 48 -i newton-%04d.png -r 48 newton.webm
# rm -f newton-*.png
