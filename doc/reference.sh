#!/bin/bash

src=$1
tag=$2

if [ -z "${tag}" ]; then
  echo "usage: $0 src tag";
  exit
fi

# traemos los defines de wasora.h para poder documentar los defaults en los detales
grep '#define' ../src/wasora.h > defs.h

kws=`grep "///${tag}+" ../src/${src}.c | awk '{print $1}' | awk -F+ '{print $2}' | sort | uniq` 

for kw in ${kws}; do

  escapedkw=`echo ${kw} | sed s/_/\\\\\\\\_/g`

  # keyword
  echo "##  ${kw}"
  echo
#   echo -n '> '
  grep "///${tag}+${kw}+desc" ../src/${src}.c | cut -d" " -f2-
  echo  

  # usage
  usage=`grep "///${tag}+${kw}+usage" ../src/${src}.c | cut -d" " -f2- | xargs`
  if [ -n "${usage}" ]; then
    echo "~~~wasora"
    echo $usage | sed s/\ \&nbsp\;\ /\\n/g
    echo "~~~"
    echo
  fi

  # math+figure
  math=`cat ../src/${src}.c | grep "///${tag}+${kw}+math"  | cut -d" " -f2-`
#   if [ -n "$math" ]; then
  if [ ! -z "" ]; then

    echo
    echo "\$\$"
    echo ${math}
    echo "\$\$"
    echo

    range=`cat ../src/${src}.c | grep "///${tag}+${kw}+plotx"  | cut -d" " -f2-`
    if [ -n "$range" ]; then
     if [[ !( -e figures/${kw}.png ) ]]; then
      cd figures
      min=`echo $range | awk '{printf $1}'` 
      max=`echo $range | awk '{printf $2}'`
      step=`echo $range | awk '{printf $3}'`
      minxtics=`echo $range | awk '{printf $4}'`
      maxxtics=`echo $range | awk '{printf $5}'`
      stepxtics=`echo $range | awk '{printf $6}'`
      minytics=`echo $range | awk '{printf $7}'`
      maxytics=`echo $range | awk '{printf $8}'`
      stepytics=`echo $range | awk '{printf $9}'`
      cat << EOF > ${kw}.was
f(x) := ${kw}(x)
PRINT_FUNCTION f MIN $min MAX $max STEP $step
EOF
        wasora ${kw}.was > ${kw}.dat
        cat << EOF > ${kw}.ppl
set preamble "\usepackage{amsmath} \usepackage{amssymb}"
set width 10*unit(cm)
set size ratio 0.35
set axis x arrow nomirrored
set axis y arrow nomirrored
# set nomytics
# set nomxtics
set nokey
EOF
      if [ ! -z "$minxtics" ]; then
        echo "set xtics $minxtics,$stepxtics,$maxxtics" >> ${kw}.ppl
      fi
      if [ ! -z "$maxytics" ]; then
        echo "set ytics $minytics,$stepytics,$maxytics" >> ${kw}.ppl
      fi

      cat << EOF >> ${kw}.ppl
set xrange [$min:$max]
set xlabel "\$x\$"
set ylabel "$escapedkw\$(x)\$"
set terminal pdf
set output "${kw}.pdf"
plot "${kw}.dat" w l lw 3 color blue
# svg does not work yet!
# set terminal svg
# set output "${kw}.svg"
# replot
set terminal png dpi 120
set output "${kw}.png"
replot
EOF
      pyxplot ${kw}.ppl
#       rm -f ${kw}.was
#       rm -f ${kw}.ppl
#       rm -f ${kw}.dat
      cd ..
     fi

    cat << EOF

![${kw}](figures/${kw})

EOF
    fi
  fi

  # detailed description
  echo
#   echo ${kw} 1>&2
  # el cut saca los tags especiales, el gcc permite usar los defines para documentar los defaults,
  # el primer sed transforma un punto solo en un newline
  # el segundo se es para poder poner links como https:/\/ (sin la barra del medio gcc piensa que es un comentario)
  grep "///${tag}+${kw}+detail" ../src/${src}.c | cut -d" " -f2- | gcc -E -P -include defs.h - | sed 's/@$//' | sed 's_/\\/_//_'
  echo  

  # examples
#   exs=`grep ///${tag}+${kw}+example ../src/${src}.c | cut -d" " -f2-` 
  exs=""
  n=0
  for ex in $exs; do
    n=$((${n} + 1))
    cat << EOF
### Example #$n, $ex
~~~wasora
EOF
      cat examples/$ex
      cat << EOF
~~~

EOF

  # terminal
    if [ -e examples/$ex.sh ]; then
      cat << EOF
~~~
EOF
      k=1
      cd examples
      rm -f $ex.term
      n=`wc -l < $ex.sh`
      while [ $k -le $n ]; do
       cat $ex.sh | head -n $k | tail -n1 > tmp
       echo -n "\$ " >> ../${src}.$ext
       cat tmp >> ../${src}.$ext
       chmod +x tmp
       script -aq -c ./tmp $ex.term 2>&1 | grep error: > errors
       if [ ! -z "`cat errors`" ]; then
         echo "error: something happened on the way to heaven"
         cat ./tmp
         cat errors
         exit 1
       fi
       k=$((${k} + 1))
      done
      cat $ex.term | grep -v Script | sed s/^\\.\$/\$/ >> ../${src}.$ext
      cd ..
      cat << EOF
$
~~~

EOF
    fi

    # figure
    if [ -e examples/$ex.fig ]; then
      echo "![$ex](examples/`cat examples/$ex.fig`)"
    fi


    echo
    echo
  done
done

echo 


rm -f examples/errors examples/tmp
