dnl This file is part of wasora and/or one of its plugins
dnl GPL v3+ (c) 2009-2016 jeremy theler
dnl <http://bitbucket.org/wasora/wasora>
dnl
changequote([,])dnl esto es para poder usar los backticks de bash
divert(-1)dnl
define([WASORA_CHECK_VCS],
if test -d ".git"; then
 if test "`which git`" = ""; then
  echo "this tree uses git but git is not installed"
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
 commitdate=`git log -1 --pretty=format:"%ad"`
 cat << EOF > src/version-vcs.h
#define WASORA_VCS_BRANCH    "${branch}"
#define WASORA_VCS_VERSION   "${version}"
#define WASORA_VCS_DATE      "${commitdate}"
#define WASORA_VCS_CLEAN     `git status --porcelain | wc -l`
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
 version=`git describe | sed 's/-/./'`
 echo "version... ${version}"

 branch=$(git symbolic-ref HEAD | sed -e 's,.*/\(.*\),\1,')
 commitdate=`git log -1 --pretty=format:"%ad"`
 author=`git log -1 --pretty=format:"%an"`
 email=`git log -1 --pretty=format:"%ae"`
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

 cat << EOF > src/version-vcs.h
#define WASORA_VCS_BRANCH    "${branch}"
#define WASORA_VCS_VERSION   "${version}"
#define WASORA_VCS_DATE      "${date}"
#define WASORA_VCS_CLEAN     `git status --porcelain | wc -l`
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

define([WASORA_README_INSTALL],
echo -n "formatting readme & install... "
if test ! -z "`which pandoc`"; then
 pandoc README.md -t plain -o README
 pandoc INSTALL.md -t plain -o INSTALL
else
 fmt -s README.md > README
 fmt -s INSTALL.md > INSTALL
fi
echo "done"
)dnl
divert(0)
