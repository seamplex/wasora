# How to generate wasora’s documentation

The documentation of wasora is contained in a set of files that act as a source for many different output formats (html, pdf, sphinx, plain text, etc) using the meta-markup tool doconce. Besides, the keywords, special variables, built-in functions, etc. are described in the very source code where they are implemented so the distance between the code that performs an integration and the description of what the parameters should be are located within two lines of distance in the same file.


## Generate the wasora logo in PDF format

    $ inkscape --export-area-drawing --export-pdf=logo.pdf logo.svg

## Have pandoc support wasora

From <https://bakerjd99.wordpress.com/2012/09/20/pandoc-based-j-syntax-highlighting/> and tweaked a little bit.

    # apt-get install cabal-install haskell-platform
    $ cabal update
    $ cabal install cabal-install
    $ ~/.cabal/bin/cabal update
    $ ~/.cabal/bin/cabal install pandoc
    $ ~/.cabal/bin/pandoc --version
    $ ~/.cabal/bin/cabal install regex-posix hxt
    $ ~/.cabal/bin/cabal unpack highlighting-kate
    $ cd highlighting-kate-0.6.2.1
    $ cp $HOME/.kde/share/apps/katepart/syntax/wasora.xml xml
    $ cp /usr/share/kde4/apps/katepart/syntax/language.dtd xml
    $ ~/.cabal/bin/cabal configure
    $ ~/.cabal/bin/cabal build
    $ runhaskell ParseSyntaxFiles.hs xml
    $ ~/.cabal/bin/cabal build
    $ ~/.cabal/bin/cabal copy
    $ ~/.cabal/bin/cabal install --force-reinstalls
    $ cd ..
    $ ~/.cabal/bin/cabal unpack pandoc
    $ cd pandoc-1.17.2
    $ ~/.cabal/bin/cabal configure
    $ ~/.cabal/bin/cabal build
    $ ~/.cabal/bin/cabal install
    $ ~/.cabal/bin/pandoc --version
