#!/bin/bash

if [ -z "`which pandoc`" ]; then
  echo "pandoc is not installed"
  exit 1 
fi

cat << EOF
---
title: Wasora showcase
subtitle: annotated (and explained) examples of inputs, execution and results
...

EOF

for inputfile in `cat inputs`; do

  echo ${inputfile} 1>&2

  # titulo
  echo
  echo -n "# "
  echo '$title$' > tmp.md
  pandoc ${inputfile}.yaml -t markdown --template tmp.md
  echo
  
  # texto
  echo
  echo '$text$' > tmp.md
  pandoc ${inputfile}.yaml -t markdown --template tmp.md
  echo

  # input 
#   echo
#   echo "## ${inputfile}.was"
#   echo
#   echo '```wasora'
#   cat ${inputfile}.was
#   echo '```     '
#   echo

cat << EOF
<div class="panel panel-default">
<div class="panel-heading" role="tab" id="heading-${inputfile}-was">
<h2 class="panel-title">
<a role="button" data-toggle="collapse" href="#collapse-${inputfile}-was" aria-expanded="false" aria-controls="collapse-${inputfile}-was">
${inputfile}.was
</a>
</h2>
</div>
<div id="collapse-${inputfile}-was" class="panel-collapse collapse" role="tabpanel" aria-labelledby="heading-${inputfile}-was">
<div class="panel-body">
    
\`\`\`wasora
EOF
  cat ${inputfile}.was
  cat << EOF
\`\`\`

</div>
</div>
</div>
EOF
  
  # terminal
  if [ -e ${inputfile}.sh ]; then
    rm -f ${inputfile}.term
    k=1
    n=`wc -l < ${inputfile}.sh`
    while [ $k -le $n ]; do
     cat ${inputfile}.sh | head -n $k | tail -n1 > tmp
     echo -n "\$ " >> ${inputfile}.term
     cat tmp >> ${inputfile}.term
     chmod +x tmp
     script -aq -c ./tmp ${inputfile}.term 2>&1 | grep error: > errors
     if [ ! -z "`cat errors`" ]; then
      echo "error: something happened on the way to heaven"
      cat ./tmp
      cat errors
      exit 1
     fi
     k=`echo $k + 1 | bc`
    done
  else
    echo "\$ wasora ${inputfile}.was" > ${inputfile}.term
    wasora ${inputfile}.was >> ${inputfile}.term
  fi
  echo "\$ " >> ${inputfile}.term

#   echo
#   echo "## Execution"
#   echo
#   echo '```'
#   cat ${inputfile}.term
#   echo '```'
#   echo

cat << EOF
<div class="panel panel-default">
<div class="panel-heading" role="tab" id="heading-${inputfile}-exec">
<h2 class="panel-title">
<a role="button" data-toggle="collapse" href="#collapse-${inputfile}-exec" aria-expanded="false" aria-controls="collapse-${inputfile}-exec">
Execution
</a>
</h2>
</div>
<div id="collapse-${inputfile}-exec" class="panel-collapse collapse" role="tabpanel" aria-labelledby="heading-${inputfile}-was-exec">
<div class="panel-body">
    
\`\`\`
EOF
  cat ${inputfile}.term
  cat << EOF
\`\`\`

</div>
</div>
</div>
EOF

  # results
  grep -q 'results:' ${inputfile}.yaml
  if [ $? -eq 0 ]; then
#     echo
#     echo "## Results"
#     echo
#     echo '$results$' > tmp.md
#     pandoc ${inputfile}.yaml -t markdown --template tmp.md
    
    cat << EOF
<div class="panel panel-default">
<div class="panel-heading" role="tab" id="heading-${inputfile}-res">
<h2 class="panel-title">
<a role="button" data-toggle="collapse" href="#collapse-${inputfile}-res" aria-expanded="false" aria-controls="collapse-${inputfile}-res">
Results
</a>
</h2>
</div>
<div id="collapse-${inputfile}-res" class="panel-collapse collapse" role="tabpanel" aria-labelledby="heading-${inputfile}-res">
<div class="panel-body">
EOF
    echo '$results$' > tmp.md
    pandoc ${inputfile}.yaml -t markdown --template tmp.md
    cat << EOF
</div>
</div>
</div>
EOF
  fi

  # explanation
  grep -q 'explanation:' ${inputfile}.yaml
  if [ $? -eq 0 ]; then
#     echo
#     echo "## Explanation"
#     echo
#     echo '$explanation$' > tmp.md
#     pandoc ${inputfile}.yaml -t markdown --template tmp.md

    cat << EOF
<div class="panel panel-default">
<div class="panel-heading" role="tab" id="heading-${inputfile}-expl">
<h2 class="panel-title">
<a role="button" data-toggle="collapse" href="#collapse-${inputfile}-expl" aria-expanded="false" aria-controls="collapse-${inputfile}-expl">
Explanation
</a>
</h2>
</div>
<div id="collapse-${inputfile}-expl" class="panel-collapse collapse" role="tabpanel" aria-labelledby="heading-${inputfile}-expl">
<div class="panel-body">
EOF
    echo '$explanation$' > tmp.md
    pandoc ${inputfile}.yaml -t markdown --template tmp.md
    cat << EOF
</div>
</div>
</div>
EOF
  fi

done

rm -f errors tmp tmp.md
