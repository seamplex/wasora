#!/bin/bash
# generate distribution packages

package="wasora"

if [ -z "$1" ]; then
 echo "usage: $0 { src | linux | cygwin | mingw }"
 exit 1
fi

arch=$1

if [ ! -e version.m4 ]; then
 echo "development tree is not configured yet"
fi

cat version.m4 | sed s/\\[/\`/g version.m4 | sed s/\\]/\'/g > origversion.m4
version=`echo wasoraversion | m4 origversion.m4 -` 
rm -f origversion.m4

html="${package}-${arch}.html"
rm -f ../${html}
n=0

if [ ${arch} = "src" ]; then
 # make source distribution
 ./autogen.sh
 ./markdown.sh || exit
 ./configure
 make dist

 # copy tarball to parent directory
 cp ${package}-${version}.tar.gz ..

 cd ..
 cat << EOF >> $html
 <li><a href="downloads/${package}-${version}.tar.gz">${package}-${version}.tar.gz</a> -
 $(echo `stat -c %s ${package}-${version}.tar.gz` / 1024 | bc) kb<br>
 ${package} main source distribution tarball
 <p>
EOF
 echo "<?php \$wasoraversion = \"${version}\"; ?>" > wasoraversion.php
 cd ${package}

elif [ ${arch} = "linux" ]; then

 n=`echo  $n + 1 | bc`
 targets[$n]="linux-amd64"
 descriptions[$n]="GNU/Linux 64-bit statically linked binary executable and source"
 configureflags[$n]=""
 optflags[$n]="-O2"
 archflags[$n]="-m64"
 linkflags[$n]="-static"
 postcmds[$n]=""

#  # gcc-multilib
#  # lib32readline6-dev
#  # hay que compilar a manopla las GSL y las IDA, copiarlas a /usr/lib32 y hacer ldconfig
#  n=`echo  $n + 1 | bc`
#  targets[$n]="linux-i386"
#  descriptions[$n]="GNU/Linux 32-bit statically linked binary executable and source"
#  configureflags[$n]=""
#  optflags[$n]="-O2"
#  archflags[$n]="-m32"
#  linkflags[$n]="-static"
#  postcmds[$n]=""

elif [ ${arch} = "cygwin" ]; then

 n=`echo  $n + 1 | bc`
 targets[$n]="win32-cygwin"
 descriptions[$n]="<a href=\"http://www.cygwin.com/\" target=\"_blank\">Cygwin</a>-based 32-bit Windows binary executable and source (includes <tt>cygwin1.dll</tt>)"
 configureflags[$n]="LIBS=-lncursesw"
 optflags[$n]="-O2"
 archflags[$n]=""
 linkflags[$n]="-static"
 postcmds[$n]="cp /usr/bin/cygwin1.dll ."

elif [ ${arch} = "mingw" ]; then

 # hay que compilar a manopla las GSL y las IDA (se necesita wine-binfmt) con
 # ./configure --host=i686-w64-mingw32 --prefix=/usr/i686-w64-mingw32
 # y hacer make install 
 n=`echo  $n + 1 | bc`
 targets[$n]="win32-mingw"
 descriptions[$n]="<a href=\"http://www.mingw.org/\" target=\"_blank\">MinGW</a>-based 32-bit Windows binary executable and source"
 configureflags[$n]="--host=i686-w64-mingw32 --prefix=/usr/i686-w64-mingw32"
 optflags[$n]="-O2"
 archflags[$n]=""
 linkflags[$n]=""
 postcmds[$n]=""

else
 echo "cannot understand architecture $1"
 exit 1
fi

if [ ! -e ../${package}-${version}.tar.gz ]; then
 echo source tarball is not ready
 exit 1
fi

cd ..

# process each target
for i in `seq 1 $n`; do

 echo $i ${targets[$i]}
 
 rm -rf ${package}-${version}-${targets[$i]}
 tar -xzf ${package}-${version}.tar.gz || exit 1
 mv ${package}-${version} ${package}-${version}-${targets[$i]} || exit 1

 cd ${package}-${version}-${targets[$i]}
 export CFLAGS="${archflags[$i]} ${optflags[$i]}"
 echo CFLAGS=$CFLAGS
 export LDFLAGS="${archflags[$i]} ${linkflags[$i]}"
 echo LDFLAGS=$LDFLAGS
 ./configure ${configureflags[$i]} > /dev/null
 make > /dev/null
 rm -f src/${package}
 if [ ! -z "${postcmds[$i]}" ]; then
  ${postcmds[$i]}
 fi
 cd ..

 tar -zcf ${package}-${version}-${targets[$i]}.tar.gz ${package}-${version}-${targets[$i]}

 cat << EOF >> $html
 <li><a href="downloads/${package}-${version}-${targets[$i]}.tar.gz">${package}-${version}-${targets[$i]}.tar.gz</a> -
 $(echo `stat -c %s ${package}-${version}-${targets[$i]}.tar.gz` / 1024 | bc) kb<br>
 ${descriptions[$i]}
 <p>
EOF

done

cd ${package}
