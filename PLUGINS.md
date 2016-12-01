Generalities about wasora plugins
=================================

These are general comments about wasora plugins and generic instructions for compiling, installing and running a wasora plugin. Particular comments should be found on the `README` file.

Plugins extend wasora's functionality by recognizing extra keywords in the input file(s) and performing particular computations (for example the plugin fino allows wasora to solve systems of partial differential equations using the finite element method and the plugin besssugo gives means to perform graphic primitives and image loading in order to build scientific videos with the results obtained by wasora). 

A plugin for wasora is a shared-object file that is loaded dynamically at runtime. There are two ways of loading a plugin, either from the commandline using the `-p` option or from the input file using the keyword `LOAD_PLUGIN`. For example, a plugin named `dummy` may be loaded from the commandline as

    $ cat test1.was
    PRINT "this is a test"
    $ wasora -p dummy test1.was
    this is a test
    $

or using the `LOAD_PLUGIN` keyword directly in the input file:

    $ cat test2.was
    LOAD_PLUGIN dummy
    PRINT "this is a test"
    $ wasora test2.was
    this is a test
    $

The argument of the `-p` option or to the `LOAD_PLUGIN` keyword should be one of the following:

 a. the name of a system-wide installed plugin (see below)
 b. a path to the shared object file. If the path contains a slash, it is treated as a (relative or absolute) pathname. Otherwise, the dynamic linker searches for the shared object file following the same rules of the kernel dynamic linker/loader (see ld.so(8) for details).

If a shared-object file using provided name as a path is not found, wasora tries the following modifications (in this order) before failing to load the plugin:

 1. prepend "./" to name, i.e ./name
 2. prepend "./" and append ".so" to name, i.e ./name.so
 3. prepend "../" to name, i.e ../name
 4. prepend "../" and append ".so" to name, i.e ../name.so
 5. append ".so" to name, i.e name.so (not a path)
 6. prepend "lib" to name, i.e. libname.so (not a path)
 7. prepend "lib" and append ".so" to name, i.e. libname.so (this is the the point where the system-wide installed plugin is looked for)


Due to the intimate interaction between wasora and its plugins, the plugin source code needs some header files from the main wasora source tree. Once a plugin is compiled, the shared object file can be loaded only from the wasora executable compiled using the very same set of common header files. Actually, an md5 sum check is performed at runtime when loading a plugin. If the checksums (of the headers used to compile wasora on the one hand and the ones used to compile the plugin on the other hand) do not match, the plugin is not be loaded and the execution fails. For example:

    $ wasora -p ./dummy.so 
    error: loading plugin ./dummy.so plugin './dummy.so' is invalid for the host, the checksums of the wasora.h header do not match:
    wasora hash: f08b94a3ee111b5eb4def6aaab1949cd
    plugin hash: 7a39bc0152161552c2b3c38c25e380ca
    Please recompile either wasora or the plugin.
    $

If the plugin can be successfully loaded, the version and a brief description should be printed:

    $ wasora -p ./dummy.so 
    dummy 0.3.6 default (2014-10-06 18:23 -0300)
    dummy plugin for wasora
    $

Several plugins can be loaded by using multiple `-p` options, multiple `LOAD_PLUGIN` keywords or a combination of both. In this case, wasora prints information about itself and about each loaded plugin:

    $ wasora -p ./foo.so -p ./bar.so
    wasora 0.4.58 trunk (2014-01-29 13:37:08 -0300 dirty)
    wasora's an advanced suite for optimization & reactor analysis


    two plugins loaded:


    plugin #1: foo 0.2.7 trunk (2014-01-29 17:03:44 -0300 dirty)
    foo plugin to perform foo actions

    plugin #2: bar 0.2.5 trunk (2014-01-16 09:20:31 -0300 dirty)
    a bar extension for wasora
    $


Depending on the size and complexity of the plugin, it may be the case (such as for example with fino and milonga) that the distribution tarball of the plugin also includes the complete source code of wasora and the makefiles can compile a standalone executable containing wasora and the plugin statically linked into it, avoiding the need of explicitly loading the shared object file. Further plugins may nevertheless be loaded, passing `-p` to the standalone executable or including the `LOAD_PLUGIN` keyword in its input.


Compiling wasora plugins
------------------------

The detailed compilation procedure is plugin-dependent (i.e. some plugin may require particular libraries). However, every wasora plugin should follow the `configure && make` procedure.


The text below the cutting line corresponds to the original FSF instructions for installing software (as wasora) that follows the GNU configure & make convention:

    $ ./configure
    $ make

Depending on the case, the configure script may provide options such as `--disable-standalone` or `--disable-plugin`. Use the `--help` option to check. The behavior of the compiled plugin may be tested with the make check step:

    $ make check

If the plugin is to be installed system-wide (so it can be loaded by passing directly "name" to the `-p` commandline option or the `LOAD_PLUGIN` keyword without bothering about paths), the make install step has to be performed as root:

    # make install

If no root access is available, the plugin can be installed user-wide by copying the the compiled shared object (i.e. `dummy.so`) to the location pointed by the `LD_LIBRARY_PATH` environment variable.

If something goes wrong and the compilation fails, please feel free to ask for help at the wasora mailing list at <wasora@seamplex.com>.
 

