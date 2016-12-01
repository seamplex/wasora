for i in . ..; do
  if [ "*.dat" != '*.dat' ]; then
    rm -f `ls $i/*.dat | grep -v histogram-samples.dat | grep -v binding_per_A.dat`
  fi
  rm -f $i/*.trs $i/*.log
  rm -f $i/*.png
  rm -f $i/*~
done
