#!/bin/bash

if [ -z "`which wasora`" ]; then
  echo "wasora not found"
  exit
fi

# el dibujito de la portada
# inkscape wasorarealbook.svg -e wasorarealbook.png -C -w 550

# la version de wasora
echo "\$ wasora -i" > version.txt
wasora -v >> version.txt
echo "\$" >> version.txt


declare -A prev
declare -A next

current_prev=""
i=1
for dir in 0*; do
 if [ -e ${dir}/README.m4 ]; then
#   echo $dir
  cases[${i}]=$dir
  next[$((${i} - 1))]=$dir
  prev[${i}]=$current_prev
  current_prev=${dir}
  i=$((${i} + 1))
 fi
done

for i in `seq 1 ${#cases[*]}`; do

 echo ${cases[${i}]}
 cd ${cases[${i}]}

 # TODO: yaml parser
#  title=`grep title README.m4 | cut -d" " -f2-`
#  tags=`grep tags README.m4 | cut -d" " -f2-`
 inputs=`grep inputs README.m4 | cut -d" " -f2-`
 difficulty=`echo ${cases[${i}]} | cut -c-3`

 if [ -e pre.sh ]; then
  ./pre.sh
 fi
  
 for input in $inputs; do
  if [ -z "$1" ]; then
   saltear=`echo ${input} | grep \~ | wc -l`
  else
   if [[ "$example" = "`basename $1`" && -z "`echo ${input} | grep \~`" ]]; then
    saltear=0
   else
    saltear=1
   fi
  fi
 
  input=`echo ${input} | sed s/~//`
  echo "    ${input}"

  if [[ $saltear -eq 0 ]]  &&  [[ -e ${input}.sh ]]; then
   rm -f terminal.txt
   j=1
   n=`wc -l < ${input}.sh`
   # TODO: usar readline
   while [ ${j} -le ${n} ]; do
    cat ${input}.sh | head -n ${j} | tail -n1 > tmp
    echo -n "\$ " >> terminal.txt
    cat tmp >> terminal.txt
    chmod +x tmp
    script -aq -c ./tmp terminal.txt 2>&1 | grep error: > errors
    if [ ! -z "`cat errors | grep -v libGL`" ]; then
     echo "error: something happened on the way to heaven"
     cat ./tmp
     cat errors
     exit 1
    else
     rm -f errors
    fi
    j=$((${j} + 1))
   done
   echo "\$ " >> terminal.txt
  fi

  if [ -e terminal.txt ]; then
   grep -v Script terminal.txt | sed s/^\\.\$/\$/ | sed '/^\s*$/d' > ${input}.term
   rm -f terminal.txt tmp
  fi
 done
 
 m4 -Dcase_difficulty=${difficulty} \
    -Dcase_prev=${prev[${i}]} \
    -Dcase_next=${next[${i}]} \
    ../header.m4 README.m4 > README.md
 
 cd ..
done
