# generates READMEs and INSTALLs if different formats from the markdown source

if [ -z "`which pandoc`" ]; then
 echo "pandoc is not installed"
 exit 1
fi

pandoc README.md -t plain -o README
pandoc INSTALL.md -t plain -o INSTALL
