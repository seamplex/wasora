rm -f *~ tmp
rm -f *.dat
rm -f *.png *.pdf
rm -f *.bmp *.avi *.webm


for i in `cat inputs`; do

  rm -f $i
  rm -f $i.term

done
