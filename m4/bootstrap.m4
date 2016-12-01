dnl This file is part of wasora and/or one of its plugins
dnl GPL v3+ (c) 2009-2016 jeremy theler
dnl <http://bitbucket.org/wasora/wasora>
dnl

changequote([,]) dnl esto es para poder usar los backticks de bash

define([WASORA_CHECK_VCS],
if test -d ".git"; then
 if test "`which git`" = ""; then
  echo "this tree uses git it is not installed" 
  exit 1
 fi
 vcs=git
else
  echo "this tree is not a repository (did you download instead of clone?)" 
  exit 1
fi
)dnl

define([WASORA_VERSION_VCS],
# wasora major version is equal to the latest tag
if test ${vcs} = "git"; then
 version=`git describe | sed 's/-/./'`
 echo "version... ${version}"
 echo "[[define]](wasoraversion, ${version})[[dnl]]" > version.m4

 branch=$(git symbolic-ref HEAD | sed -e 's,.*/\(.*\),\1,')
 date=`git log --pretty=format:"%ad" | head -n1`
 cat << EOF > src/version-vcs.h
#define WASORA_VCS_BRANCH    \"{branch}\"\n\
#define WASORA_VCS_VERSION   \"${version}\"\n\
#define WASORA_VCS_DATE      \"${date}\"\n\
#define WASORA_VCS_CLEAN     `git status --porcelain | wc -l`\n" > src/version-vcs.h
EOF

 echo -n "building changelog... "
 git log > ChangeLog
 echo "done"

else
 majorversion=x.y
 version=${majorversion}.z
 touch ChangeLog
fi
)dnl

define([PLUGIN_VERSION_VCS],
# plugin version
if test "${vcs}" = "git"; then
 echo -n "major version... "
 majorversion=`git log -r tip --template='{latesttag}'`
 echo ${majorversion}
 
 echo -n "minor version... "
 minorversion=`git log -r tip --template="{latesttagdistance}"`
 echo ${minorversion}
 
 version="${majorversion}.${minorversion}"
 commitdate=`git log -r tip --template="{date|shortdate}"`
 author=`git log -r tip --template="{author|person}"`
 email=`git log -r tip --template="{author|email}"`
 quotedemail=`echo ${email} | sed s/@/@@/`
 year=`echo ${shortdate} | cut -c-4`

cat << EOF > plugin.mk
plugin = ${plugin}
EOF
 
cat << EOF > version.m4
[[define]]([pluginname], [${plugin}])[[dnl]]
[[define]]([pluginversion], [${version}])[[dnl]]
[[define]]([authoremail], [${email}])[[dnl]]
EOF

cat << EOF > doc/version.texi
@set UPDATED ${commitdate}
@set UPDATED-MONTH ${commitdate}
@set EDITION ${version}
@set VERSION ${version}
EOF

cat << EOF > doc/plugin.texi
@set PLUGIN  ${plugin}
@set AUTHOR  ${author}
@set EMAIL   ${quotedemail}
@set YEAR    ${year}
EOF

 git log -r tip --template="\
#define PLUGIN_VCS_BRANCH    \"{branch}\"\n\
#define PLUGIN_VCS_MAJOR     \"${majorversion}\"\n\
#define PLUGIN_VCS_MINOR     \"${minorversion}\"\n\
#define PLUGIN_VCS_REVID     \"{node}\"\n\
#define PLUGIN_VCS_SHORTID   \"{node|short}\"\n\
#define PLUGIN_VCS_REVNO     {rev}\n\
#define PLUGIN_VCS_DATE      \"{date|isodate}\"\n\
#define PLUGIN_VCS_CLEAN     `git status | wc -l`\n" > src/version-vcs.h

 echo -n "building changelog... "
 git log --style changelog > ChangeLog
 echo "done"
else
 majorversion=x.y
 version=${majorversion}.z
 touch ChangeLog
fi
)dnl

define([WASORA_README_INSTALL],
echo -n "formatting readme & install... "
if test ! -z "`which pandoc`"; then
# TODO: yaml + hash
 ./markdown.sh
else
 fmt -s README.md > README
 fmt -s INSTALL.md > INSTALL
fi
# these are needed by make so we touch them
touch doc/README.pdf doc/README.html
touch doc/INSTALL.pdf doc/INSTALL.html
touch doc/wasora_full.html
echo "done"
)dnl

define([PLUGIN_README_INSTALL],
echo -n "formatting readme & install... "
if test ! -z "`which pandoc`"; then
 ./markdown.sh 
else
 fmt -s README.md > README
 fmt -s INSTALL.md > INSTALL
 fmt -s ${WASORA_PATH}/PLUGINS.md > PLUGINS
fi
# these are needed by make so we touch them
touch doc/README.pdf doc/README.html
touch doc/INSTALL.pdf doc/INSTALL.html
touch doc/PLUGINS.pdf doc/PLUGINS.html
echo "done"
)dnl

define([PLUGIN_FIND_WASORA],
# locate wasora source dir, first
if test ! -z "${1}"; then
 export WASORA_PATH=${1}
elif test ! -z "${WASORA_DIR}"; then
 export WASORA_PATH=${WASORA_DIR}
else
 TESTDIRS="../wasora ../../wasora ../../../wasora"
 for i in ${TESTDIRS}; do
  if test -z "${WASORA_PATH}" -a -d $i; then 
   export WASORA_PATH=$i
  fi
 done
 if test -z "${WASORA_PATH}"; then
  cat << EOF
cannot find wasora source tree in
${TESTDIRS}
please either set the WASORA_DIR environment variable
or give [[$]][[0]] an argument where the wasora source tree is.
See the PLUGINS file for further details.
EOF
  exit 1
 fi
fi

echo "using wasora source from "${WASORA_PATH}
if test ! -e "${WASORA_PATH}/src/wasora.h"; then
 echo "error: cannot find ${WASORA_PATH}/src/wasora.h"
 exit 1
fi

# copy wasora
echo -n "copying wasora source... "
rm -rf src/wasora
mkdir src/wasora
cp -r ${WASORA_PATH}/src/* src/wasora
rm -f src/wasora/wasora src/wasora/Makefile* src/wasora/*.o  src/wasora/*.lo src/wasora/*.lo
if test ! -f src/wasora/version-conf.h; then
 echo "error: the wasora tree at ${WASORA_PATH} is not configured yet"
 echo "execute ./configure (previously ./autogen.sh if needed) at ${WASORA_PATH}" 
 exit 1
fi
cd src/wasora
./version.sh
cd ../..
echo "done"
)dnl

define([PLUGIN_COPY_M4],
# tautologically copy al m4 (including wasora's bootstrap.m4!)
cp ${WASORA_PATH}/m4/*.m4 ./m4
)dnl

define([WASORA_AUTOCLEAN],
if test -e Makefile; then
  make clean
fi
rm -f *~ .*~ src/*~
rm -rf src/.libs
rm -f src/version.h src/version-vcs.h src/version-conf.h
rm -f README INSTALL PLUGINS ChangeLog
rm -f version.m4
rm -f doc/README.pdf doc/README.html doc/INSTALL.pdf doc/INSTALL.html doc/PLUGINS.pdf doc/PLUGINS.html
rm -f aclocal.m4 configure config.log config.status config.guess config.sub compile depcomp install-sh missing configure.scan autoscan.log test-driver
rm -f Makefile Makefile.in src/Makefile src/Makefile.in
rm -rf autom4te.cache 

cd examples
./clean.sh
cd ..
)dnl

define([PLUGIN_AUTOCLEAN],
if test -e Makefile; then
  make clean
fi
rm -f *~ src/*~ src/thirdparty/*~
rm -f src/*.o src/*.lo src/*~
rm -rf src/.deps src/.libs src/.dirstamp src/stamp-h1 src/config.h.in src/config.h
rm -f README INSTALL PLUGINS README.pdf INSTALL.pdf PLUGINS.pdf README.html INSTALL.html PLUGINS.html ChangeLog
rm -f README doc/README.pdf doc/README.html INSTALL doc/INSTALL.pdf doc/INSTALL.html PLUGINS doc/PLUGINS doc/PLUGINS.pdf doc/PLUGINS.html ChangeLog
rm -f aclocal.m4 configure config.log config.status compile depcomp install-sh missing ltmain.sh config.guess config.sub libtool libtool test-driver
rm -f src/version.h src/version-vcs.h src/version-conf.h
rm -rf autom4te.cache
rm -rf src/.deps src/.libs src/.dirstamp src/stamp-h1 src/config.h.in src/config.h
rm -f Makefile Makefile.in src/Makefile src/Makefile.in
rm -rf ${plugin}.so ${plugin} src/lib${plugin}.la 
rm -rf src/wasora
cd examples
./clean.sh
cd ..
)dnl
