% Wasora v0.6.23-g5154b2c reference sheet

# Keywords

##  .=

Add an equation to the DAE system to be solved in the phase space spanned by `PHASE_SPACE`.

~~~wasora
{ 0[(i[,j]][<imin:imax[;jmin:jmax]>] | <expr1> } .= <expr2>
~~~



##  =

Assign an expression to a variable, a vector or a matrix.

~~~wasora
<var>[ [<expr_tmin>, <expr_tmax>] | @<expr_t> ] = <expr> <vector>(<expr_i>)[<expr_i_min, expr_i_max>] [ [<expr_tmin>, <expr_tmax>] | @<expr_t> ] = <expr> <matrix>(<expr_i>,<expr_j>)[<expr_i_min, expr_i_max; expr_j_min, expr_j_max>] [ [<expr_tmin>, <expr_tmax>] | @<expr_t> ] = <expr>
~~~



##  ABORT

Catastrophically abort the execution and quit wasora.

~~~wasora
ABORT
~~~


Whenever the instruction `ABORT` is executed, wasora quits without closing files
or unlocking shared memory objects. The objective of this instruction is, as
illustrated in the examples, either to debug complex input files and check the
values of certain variables or to conditionally abort the execution using `IF`
clauses.

##  ALIAS

Define a scalar alias of an already-defined indentifier.

~~~wasora
ALIAS { <new_var_name> IS <existing_object> | <existing_object> AS <new_name> }
~~~


The existing object can be a variable, a vector element or a matrix element.
In the first case, the name of the variable should be given as the existing object.
In the second case, to alias the second element of vector `v` to the new name `new`, `v(2)` should be given as the existing object.
In the third case, to alias second element (2,3) of matrix `M` to the new name `new`, `M(2,3)` should be given as the existing object.

##  CALL

Call a previously dynamically-loaded user-provided routine.

~~~wasora
CALL <name> [ expr_1 expr_2 ... expr_n ]
~~~



##  CLOSE

Explicitly close an already-`OPEN`ed file.

~~~wasora
CLOSE
~~~



##  CONST

Mark a scalar variable, vector or matrix as a constant.

~~~wasora
CONST name_1 [ <name_2> ] ... [ <name_n> ]
~~~



##  DEFAULT_ARGUMENT_VALUE

Give a default value for an optional commandline argument.

~~~wasora
DEFAULT_ARGUMENT_VALUE <constant> <string>
~~~


If a `$n` construction is found in the input file but the
commandline argument was not given, the default behavior is to
fail complaining that an extra argument has to be given in the
commandline. With this keyword, a default value can be assigned if
no argument is given, thus avoiding the failure and making the argument
optional.

##  DIFFERENTIAL

Explicitly mark variables, vectors or matrices as “differential” to compute intial conditions of DAE systems.

~~~wasora
DIFFERENTIAL { <var_1> <var_2> ... | <vector_1> <vector_2> ... | <matrix_1> <matrix_2> ... }
~~~



##  DO_NOT_EVALUATE_AT_PARSE_TIME

Ask wasora not to evaluate assignments at parse time.

~~~wasora
DO_NOT_EVALUATE_AT_PARSE_TIME
~~~



##  FILE

Define a file, either as input or as output, for further usage.

~~~wasora
< FILE | OUTPUT_FILE | INPUT_FILE > <name> <printf_format> [ expr_1 expr_2 ... expr_n ] [ INPUT | OUTPUT | MODE <fopen_mode> ] [ OPEN | DO_NOT_OPEN ]
~~~



##  FIT

Fit a function of one or more arguments to a set of data.

~~~wasora
FIT <function_to_be_fitted> TO <function_with_data> VIA <var_1> <var_2> ... <var_n> [ GRADIENT <expr_1> <expr_2> ... <expr_n> ] [ RANGE_MIN <expr_1> <expr_2> ... <expr_n> ] [ RANGE_MAX <expr_1> <expr_2> ... <expr_n> ] [ DELTAEPSREL <expr> ] [ DELTAEPSABS <expr> ] [ MAX_ITER <expr> ] [ VERBOSE ] [ RERUN | DO_NOT_RERUN ]
~~~


The function with the data has to be point-wise defined.
The function to be fitted hast to be parametrized with at least one of the variables provided after the `VIA` keyword.
Only the names of the functions have to be given.
Both functions have to have the same number of arguments.
The initial guess of the solution is given by the initial value of the variables listed in the `VIA` keyword.
Analytical expressions for the gradient of the function to be fitted with respect to the parameters to be fitted can be optionally given with the `GRADIENT` keyword.
If none is provided, the gradient will be computed numerically using finite differences.
A range over which the residuals are to be minimized can be given with `RANGE_MIN` and `RANGE_MAX`.
For multidimensional fits, the range is an hypercube.
If no range is given, all the definition points of the function witht the data are used for the fit.
Convergence can be controlled by given the relative and absolute tolreances with
`DELTAEPSREL` (default 1e-4) and `DELTAEPSABS` (default 1e-6),
and with the maximum number of iterations `MAX_ITER` (default 100).
If the optional keyword `VERBOSE` is given, some data of the intermediate steps is written in the standard output.

##  FUNCTION

Define a function of one or more variables.

~~~wasora
FUNCTION <name>(<var_1>[,var2,...,var_n]) { [ = <expr> | FILE_PATH <file_path> | ROUTINE <name> | | MESH <name> { DATA <new_vector_name> | VECTOR <existing_vector_name> } { NODES | CELLS } | [ VECTOR_DATA <vector_1> <vector_2> ... <vector_n> <vector_n+1> ] } [COLUMNS <expr_1> <expr_2> ... <expr_n> <expr_n+1> ] [ INTERPOLATION { linear | polynomial | spline | spline_periodic | akima | akima_periodic | steffen | nearest | shepard | modified_shepard | bilinear } ] [ INTERPOLATION_THRESHOLD <expr> ] [ SHEPARD_RADIUS <expr> ] [ SHEPARD_EXPONENT <expr> ] [ SIZES <expr_1> <expr_2> ... <expr_n> ] [ X_INCREASES_FIRST <expr> ] [ DATA <num_1> <num_2> ... <num_N> ]
~~~


The number of variables $n$ is given by the number of arguments given between parenthesis after the function name.
The arguments are defined as new variables if they had not been already defined as variables.
If the function is given as an algebraic expression, the short-hand operator `:=` can be used.
That is to say, `FUNCTION f(x) = x^2` is equivalent to `f(x) := x^2`.
If a `FILE_PATH` is given, an ASCII file containing at least $n+1$ columns is expected.
By default, the first $n$ columns are the values of the arguments and the last column is the value of the function at those points.
The order of the columns can be changed with the keyword `COLUMNS`, which expects $n+1$ expressions corresponding to the column numbers.
A function of type `ROUTINE` calls an already-defined user-provided routine using the `CALL` keyword and passes the values of the variables in each required evaluation as a `double *` argument.
If `MESH` is given, the definition points are the nodes or the cells of the mesh.
The function arguments should be $(x)$, $(x,y)$ or $(x,y,z)$ matching the dimension the mesh.
If the keyword `DATA` is used, a new empty vector of the appropriate size is defined.
The elements of this new vector can be assigned to the values of the function at the $i$-th node or cell.
If the keyword `VECTOR` is used, the values of the dependent variable are taken to be the values of the already-existing vector.
Note that this vector should have the size of the number of nodes or cells the mesh has, depending on whether `NODES` or `CELLS` is given.
If `VECTOR_DATA` is given, a set of $n+1$ vectors of the same size is expected.
The first $n+1$ correspond to the arguments and the last one is the function value.
Interpolation schemes can be given for either one or multi-dimensional functions with `INTERPOLATION`.
Available schemes for $n=1$ are:

 * linear
 * polynomial, the grade is equal to the number of data minus one
 * spline, cubic (needs at least 3 points)
 * spline_periodic
 * akima (needs at least 5 points)
 * akima_periodic (needs at least 5 points)
 * steffen, always-monotonic splines-like (available only with GSL >= 2.0)

Default interpolation scheme for one-dimensional functions is `(*gsl_interp_linear)`.

Available schemes for $n>1$ are:

 * nearest, $f(\vec{x})$ is equal to the value of the closest definition point
 * shepard, weighted average of close definition points
 * modified_shepard, weighted average of close definition points
 * bilinear, only available if the definition points configure an structured hypercube-like grid. If $n>2$, `SIZES` should be given.

For $n>1$, if the euclidean distance between the arguments and the definition points is smaller than `INTERPOLATION_THRESHOLD`, the definition point is returned and no interpolation is performed.
Default value is square root of `9.5367431640625e-07`.
The radius of points to take into account in `shepard` is given by `SHEPARD_RADIUS`.
Default is `1.0`.
The exponent of the `shepard` method is given by `SHEPARD_EXPONENT`.
Default is `2`.
When requesting `bilinear` interpolation for $n>2$, the number of definition points for each argument variable has to be given with `SIZES`,
and wether the definition data is sorted with the first argument changing first (`X_INCREASES_FIRST` evaluating to non-zero) or with the last argument changing first (zero).
The function can be pointwise-defined inline in the input using `DATA`. This should be the last keyword of the line, followed by $N=k\cdot (n+1)$ expresions giving $k$ definition points: $n$ arguments and the value of the function.
Multiline continuation using brackets `{` and `}` can be used for a clean data organization. See the examples.

##  HISTORY

Record the time history of a variable as a function of time.

~~~wasora
HISTORY <variable> <function>
~~~



##  IF

Begin a conditional block.

~~~wasora
IF expr
<block_of_instructions_if_expr_is_true>
[ ELSE ]
[block_of_instructions_if_expr_is_false]
ENDIF
~~~



##  IMPLICIT

Define whether implicit declaration of variables is allowed or not.

~~~wasora
IMPLICIT { NONE | ALLOWED }
~~~


By default, wasora allows variables (but not vectors nor matrices) to be
implicitly declared. To avoid introducing errors due to typos, explicit
declaration of variables can be forced by giving `IMPLICIT NONE`.
Whether implicit declaration is allowed or explicit declaration is required
depends on the last `IMPLICIT` keyword given, which by default is `ALLOWED`.

##  INCLUDE

Include another wasora input file.

~~~wasora
INCLUDE <file_path> [ FROM <num_expr> ] [ TO <num_expr> ]
~~~


Includes the input file located in the string `file_path` at the current location.
The effect is the same as copying and pasting the contents of the included file
at the location of the `INCLUDE` keyword. The path can be relative or absolute.
Note, however, that when including files inside `IF` blocks that instructions are
conditionally-executed but all definitions (such as function definitions) are processed at
parse-time independently from the evaluation of the conditional.
The optional `FROM` and `TO` keywords can be used to include only portions of a file.

##  INITIAL_CONDITIONS_MODE

Define how initial conditions of DAE problems are computed.

~~~wasora
INITIAL_CONDITIONS_MODE { AS_PROVIDED | FROM_VARIABLES | FROM_DERIVATIVES }
~~~


In DAE problems, initial conditions may be either:

 * equal to the provided expressions (`AS_PROVIDED`)
 * the derivatives computed from the provided phase-space variables (`FROM_VARIABLES`)
 * the phase-space variables computed from the provided derivatives (`FROM_DERIVATIVES`)

In the first case, it is up to the user to fulfill the DAE system at\ $t = 0$.
If the residuals are not small enough, a convergence error will occur.
The `FROM_VARIABLES` option means calling IDA’s `IDACalcIC` routine with the parameter `IDA_YA_YDP_INIT`.
The `FROM_DERIVATIVES` option means calling IDA’s `IDACalcIC` routine with the parameter IDA_Y_INIT.
Wasora should be able to automatically detect which variables in phase-space are differential and
which are purely algebraic. However, the `DIFFERENTIAL` keyword may be used to explicitly define them.
See the (SUNDIALS documentation)[https://computation.llnl.gov/casc/sundials/documentation/ida_guide.pdf] for further information.

##  LOAD_PLUGIN

Load a wasora plug-in from a dynamic shared object.

~~~wasora
LOAD_PLUGIN { <file_path> | <plugin_name> }
~~~


A wasora plugin in the form of a dynamic shared object (i.e. `.so`) can be loaded
either with the `LOAD_PLUGIN` keyword or from the command line with the `-p` option.
Either a file path or a plugin name can be given. The following locations are tried:

 * the current directory `./`
 * the parent directory `../`
 * the user’s `LD_LIBRARY_PATH`
 * the cache file `/etc/ld.so.cache`
 * the directories `/lib`, `/usr/lib`, `/usr/local/lib`

If a wasora plugin was compiled and installed following the `make install` procedure,
the plugin should be loaded by just passing the name to `LOAD_PLUGIN`.

##  LOAD_ROUTINE

Load one or more routines from a dynamic shared object.

~~~wasora
LOAD_ROUTINE <file_path> <routine_1> [ <routine_2> ... <routine_n> ]
~~~



##  M4

Call the `m4` macro processor with definitions from wasora variables or expressions.

~~~wasora
M4 { INPUT_FILE <file_id> | FILE_PATH <file_path> } { OUTPUT_FILE <file_id> | OUTPUT_FILE_PATH <file_path> } [ EXPAND <name> ] ... } [ MACRO <name> [ <format> ] <definition> ] ... }
~~~



##  MATRIX

Define a matrix.

~~~wasora
MATRIX <name> ROWS <expr> COLS <expr> [ DATA num_expr_1 num_expr_2 ... num_expr_n ]
~~~



##  MINIMIZE

Find the combination of arguments that give a (relative) minimum of a function, i.e. run an optimization problem.

~~~wasora
MINIMIZE <function> <function> [ METHOD { conjugate_fr | conjugate_pr | vector_bfgs2 | vector_bfgs | steepest_descent | nmsimplex2 | nmsimplex | nmsimplex2rand } [ GRADIENT <expr_1> <expr_2> ... <expr_n> ] [ GUESS <expr_1> <expr_2> ... <expr_n> ] [ MIN <expr_1> <expr_2> ... <expr_n> ] [ MAX <expr_1> <expr_2> ... <expr_n> ] [ STEP <expr_1> <expr_2> ... <expr_n> ] [ VERBOSE ] [ NORERUN ] [ MAX_ITER <expr> ] [ TOL <expr> ] [ GRADTOL <expr> ]
~~~



##  PARAMETRIC

Systematically sweep a zone of the parameter space, i.e. perform a parametric run.

~~~wasora
PARAMETRIC <var_1> [ ... <var_n> ] [ TYPE { linear | logarithmic | random | gaussianrandom | sobol | niederreiter | halton | reversehalton } ] [ MIN <num_expr_1> ... <num_expr_n> ] [ MAX <num_expr_1> ... <num_expr_n> ] [ STEP <num_expr_1> ... <num_expr_n> ] [ NSTEPS <num_expr_1> ... <num_expr_n> ] [ OUTER_STEPS <num_expr> ] [ MAX_DAUGHTERS <num_expr> ] [ OFFSET <num_expr> ] [ ADIABATIC ]
~~~



##  PHASE_SPACE

Define which variables, vectors and/or matrices belong to the phase space of the DAE system to be solved.

~~~wasora
PHASE_SPACE { <vars> | <vectors> | <matrices> }
~~~



##  PRINT

Print plain-text and/or formatted data to the standard output or into an output file.

~~~wasora
PRINT [ FILE <file_id> | FILE_PATH <file_path> ] [ NONEWLINE ] [ SEP <string> ] [ NOSEP ] [ HEADER ] [ SKIP_STEP <expr> ] [ SKIP_STATIC_STEP <expr> ] [ SKIP_TIME <expr> ] [ SKIP_HEADER_STEP <expr> ] [ <object_1> <object_2> ... <object_n> ] [ TEXT <string_1> ... TEXT <string_n> ]
~~~


Each argument `object` that is not a keyword is expected to be part of the output, can be either a matrix, a vector, an scalar algebraic expression.
If the given object cannot be solved into a valid matrix, vector or expression, it is treated as a string literal if `IMPLICIT` is `ALLOWED`, otherwise a parser error is raised.
To explicitly interpret an object as a literal string even if it resolves to a valid numerical expression, it should be prefixed with the `TEXT` keyword.
Hashes `#` appearing literal in text strings have to be quoted to prevent the parser to treat them as comments within the wasora input file and thus ignoring the rest of the line.
Whenever an argument starts with a porcentage sign `%`, it is treated as a C `printf`-compatible format definition and all the objects that follow it are printed using the given format until a new format definition is found.
The objects are treated as double-precision floating point numbers, so only floating point formats should be given. The default format is `"%g"`.
Matrices, vectors, scalar expressions, format modifiers and string literals can be given in any desired order, and are processed from left to right.
Vectors are printed element-by-element in a single row. See `PRINT_VECTOR` to print vectors column-wise.
Matrices are printed element-by-element in a single line using row-major ordering if mixed with other objects but in the natural row and column fashion if it is the only given object.
If the `FILE` keyword is not provided, default is to write to stdout.
If the `NONEWLINE` keyword is not provided, default is to write a newline '\n' character after all the objects are processed.
The `SEP` keywords expects a string used to separate printed objects, the default is a tab 'DEFAULT_PRINT_SEPARATOR' character.
Use the `NOSEP` keyword to define an empty string as object separator.
If the `HEADER` keyword is given, a single line containing the literal text
given for each object is printed at the very first time the `PRINT` instruction is
processed, starting with a hash `#` character.
If the `SKIP_STEP` (`SKIP_STATIC_STEP`)keyword is given, the instruction is processed only every
the number of transient (static) steps that results in evaluating the expression,
which may not be constant. By default the `PRINT` instruction is evaluated every
step. The `SKIP_HEADER_STEP` keyword works similarly for the optional `HEADER` but
by default it is only printed once. The `SKIP_TIME` keyword use time advancements
to choose how to skip printing and may be useful for non-constant time-step problems.

##  PRINT_FUNCTION

Print one or more functions as a table of values of dependent and independent variables.

~~~wasora
PRINT_FUNCTION <function_1> [ { function_2 | expr_1 } ... { function_n | expr_n-1 } ] [ FILE <file_id> | FILE_PATH <file_path> ] [ HEADER ] [ MIN <expr_1> <expr_2> ... <expr_m> ] [ MAX <expr_1> <expr_2> ... <expr_m> ] [ STEP <expr_1> <expr_2> ... <expr_m> ] [ NSTEPs <expr_1> <expr_2> ... <expr_m> ] [ FORMAT <print_format> ] [ PHYSICAL_ENTITY <name> ]
~~~



##  PRINT_VECTOR

Print the elements of one or more vectors.

~~~wasora
PRINT_VECTOR [ FILE <file_id> ] FILE_PATH <file_path> ] [ { VERTICAL | HORIZONTAL } ] [ ELEMS_PER_LINE <expr> ] [ FORMAT <print_format> ] <vector_1> [ vector_2 ... vector_n ]
~~~



##  READ

Read data (variables, vectors o matrices) from files or shared-memory segments.

~~~wasora
[ READ | WRITE ] [ SHM <name> ] [ { ASCII_FILE_PATH | BINARY_FILE_PATH } <file_path> ] [ { ASCII_FILE | BINARY_FILE } <identifier> ] [ IGNORE_NULL ] [ object_1 object_2 ... object_n ]
~~~



##  SEMAPHORE

Perform either a wait or a post operation on a named shared semaphore.

~~~wasora
[ SEMAPHORE | SEM ] <name> { WAIT | POST }
~~~



##  SHELL

Execute a shell command.

~~~wasora
SHELL <print_format> [ expr_1 expr_2 ... expr_n ]
~~~



##  SOLVE

Solve a non-linear system of\ $n$ equations with\ $n$ unknowns.

~~~wasora
SOLVE <n> UNKNOWNS <var_1> <var_2> ... <var_n> RESIDUALS <expr_1> <expr_2> ... <expr_n> ] GUESS <expr_1> <expr_2> ... <expr_n> ] [ METHOD { dnewton | hybrid | hybrids | broyden } ] [ EPSABS <expr> ] [ EPSREL <expr> ] [ MAX_ITER <expr> ] [ VERBOSE ]
~~~



##  TIME_PATH

Force transient problems to pass through specific instants of time.

~~~wasora
TIME_PATH <expr_1> [ <expr_2> [ ... <expr_n> ] ]
~~~


The time step `dt` will be reduced whenever the distance between
the current time `t` and the next expression in the list is greater
than `dt` so as to force `t` to coincide with the expressions given.
The list of expresssions should evaluate to a sorted list of values.

##  VAR

Define one or more scalar variables.

~~~wasora
VAR <name_1> [ <name_2> ] ... [ <name_n> ]
~~~



##  VECTOR

Define a vector.

~~~wasora
VECTOR <name> SIZE <expr> [ DATA <expr_1> <expr_2> ... <expr_n> | FUNCTION_DATA <function> ]
~~~



##  WRITE

Write data (variables, vectors o matrices) to files or shared-memory segments.
See the `READ` keyword for usage details.






--------------

# Mesh-related keywords

##  MATERIAL


~~~wasora
MATERIAL <name> [ MESH <name> ] [ PHYSICAL_ENTITY <name_1> [ PHYSICAL_ENTITY <name_2> [ ... ] ] ] [ <property_name_1> <expr_1> [ <property_name_2> <expr_2> [ ... ] ] ]
~~~



##  MESH


~~~wasora
MESH [ NAME <name> ] [ FILE <file_id> | FILE_PATH <file_path> ] [ STRUCTURED ] [ DIMENSIONS <num_expr> ] [ ORDERING { unknown | node } ] [ SCALE <expr> ] [ OFFSET <expr_x> <expr_y> <expr_z>] [ DEGREES <num_expr> ] [ NCELLS_X <expr> ] [ NCELLS_Y <expr> ] [ NCELLS_Z <expr> ] [ LENGTH_X <expr> ] [ LENGTH_Y <expr> ] [ LENGTH_Z <expr> ] [ DELTA_X <expr> ] [ DELTA_Y <expr> ] [ DELTA_Z <expr> ] ///kw+MESH+usage [ READ_FUNCTION <function_name> ] [...] [ READ_SCALAR <name_in_mesh> AS <function_name> ] [...]
~~~



##  MESH_FILL_VECTOR


~~~wasora
MESH_FILL_VECTOR [ MESH <name> ] [ NODES | CELLS ] VECTOR <vector> { FUNCTION <function> | EXPRESSION <expr> }
~~~



##  MESH_FIND_MAX


~~~wasora
MESH_FIND_MAX { FUNCTION <function> | EXPRESSION <expr> } [ MESH <name> ] [ PHYSICAL_ENTITY <physical_entity_name> ] [ NODES | CELLS ] [ MAX <variable> ] [ I_MAX <variable> ] [ X_MAX <variable> ] [ Y_MAX <variable> ] [Z_MAX <variable> ]
~~~



##  MESH_INTEGRATE


~~~wasora
MESH_INTEGRATE { FUNCTION <function> | EXPRESSION <expr> } [ MESH <mesh_identifier> ] [ OVER <physical_entity_name> ] RESULT <variable> [ NODES | CELLS ] [ GAUSS_POINTS <num_expr> ]
~~~



##  MESH_MAIN


~~~wasora
MESH_MAIN [ <name> ]
~~~



##  MESH_POST


~~~wasora
MESH_POST [ MESH <mesh_identifier> ] { FILE <name> | FILE_PATH <file_path> } [ NO_MESH ] [ FORMAT { gmsh | vtk } ] [ CELLS | ] NODES ] [ NO_PHYSICAL_NAMES ] [ VECTOR <function1_x> <function1_y> <function1_z> ] [...] [ <scalar_function_1> ] [ <scalar_function_2> ] ...
~~~



##  PHYSICAL_ENTITY


~~~wasora
PHYSICAL_ENTITY <name> [ DIMENSION <expr> ] [ MESH <name> ] [ MATERIAL <name> ] [ BC <bc_1> <bcg_2> ... ]
~~~



##  PHYSICAL_PROPERTY


~~~wasora
PHYSICAL_PROPERTY <name> [ <material_name1> <expr1> [ <material_name2> <expr2> ] ... ]
~~~






--------------

# Variables

##  done

Flag that indicates whether the overall calculation is over.



##  done_outer

Flag that indicates whether the parametric, optimization of fit calculation is over or not.
It is set to true (i.e. $\neq 0$) by wasora whenever the outer calculation is considered to be finished,
which can be that the parametric calculation swept the desired parameter space or that the
optimization algorithm reached the desired convergence criteria.
If the user sets it to true, the current step is marked as the last outer step and
the transient calculation ends after finishing the step.



##  done_static

Flag that indicates whether the static calculation is over or not.
It is set to true (i.e. $\neq 0$) by wasora if `step_static` $\ge$ `static_steps`.
If the user sets it to true, the current step is marked as the last static step and
the static calculation ends after finishing the step.



##  done_transient

Flag that indicates whether the transient calculation is over or not.
It is set to true (i.e. $\neq 0$) by wasora if `t` $\ge$ `end_time`.
If the user sets it to true, the current step is marked as the last transient step and
the transient calculation ends after finishing the step.



##  dt

Actual value of the time step for transient calculations. When solving DAE systems,
this variable is set by wasora. It can be written by the user for example by importing it from another
transient code by means of shared-memory objects. Care should be taken when
solving DAE systems and overwriting `t`. Default value is 1/16, which is
a power of two and roundoff errors are thus reduced.



##  end_time

Final time of the transient calculation, to be set by the user. 
The default value is zero, meaning no transient calculation.



##  i

Dummy index, used mainly in vector and matrix row subindex expressions.



##  infinite

A very big positive number, which can be used as `end_time = infinite` or
to define improper integrals with infinite limits. Default is $2^{50} \approx 1 \times 10^{15}$.



##  in_outer_initial

Flag that indicates if the current step is the initial step of an optimization of fit run.



##  in_static

Flag that indicates if wasora is solving the iterative static calculation.
Flag that indicates if wasora is in the first step of the iterative static calculation.
Flag that indicates if wasora is in the last step of the iterative static calculation.



##  in_transient

Flag that indicates if wasora is solving transient calculation.



##  in_transient_first

Flag that indicates if wasora is in the first step of the transient calculation.



##  in_transient_last

Flag that indicates if wasora is in the last step of the transient calculation.



##  j

Dummy index, used mainly in matrix column subindex expressions.



##  max_dt

Maximum bound for the time step that wasora should take when solving DAE systems.



##  min_dt

Minimum bound for the time step that wasora should take when solving DAE systems.



##  ncores

The number of online available cores, as returned by `sysconf(_SC_NPROCESSORS_ONLN)`.
This value can be used in the `MAX_DAUGHTERS` expression of the `PARAMETRIC` keyword
(i.e `ncores/2`).



##  on_gsl_error

This should be set to a mask that indicates how to proceed if an error ir raised in any
routine of the GNU Scientific Library. 



##  on_ida_error

This should be set to a mask that indicates how to proceed if an error ir raised in any
routine of the SUNDIALS IDA Library. 



##  on_nan

This should be set to a mask that indicates how to proceed if Not-A-Number signal (such as a division by zero)
is generated when evaluating any expression within wasora.



##  pi

A double-precision floating point representaion of the number $\pi$, equal to
`math.h` 's `M_PI` constant.



##  pid

The UNIX process id of wasora (or the plugin).



##  realtime_scale

If this variable is not zero, then the transient problem is run trying to syncrhonize the
problem time with realtime, up to a scale given. For example, if the scale is set to one, then
wasora will advance the problem time at the same pace that the real wall time advances. If set to
two, wasora's time wil advance twice as fast as real time, and so on. If the calculation time is
slower than real time modified by the scale, this variable has no effect on the overall behavior
and execution will proceed as quick as possible with no delays.



##  rel_error

Maximum allowed relative error for the solution of DAE systems. Default value is
is $1 \times 10^{-6}$. If a fine per-variable error control is needed, special vector
`abs_error` should be used.



##  static_steps

Number of steps that ought to be taken during the static calculation, to be set by the user. 
The default value is one, meaning only one static step. 



##  step_outer

Indicates the current step number of the iterative outer calculation (parametric, optimization or fit).
Indicates the current step number of the iterative inner calculation (optimization or fit).



##  step_static

Indicates the current step number of the iterative static calculation.



##  step_transient

Indicates the current step number of the transient static calculation.



##  t

Actual value of the time for transient calculations. This variable is set by
wasora, but can be written by the user for example by importing it from another
transient code by means of shared-memory objects. Care should be taken when
solving DAE systems and overwriting `t`.



##  zero

A very small positive number, which is taken to avoid roundoff 
errors when comparing floating point numbers such as replacing $a \leq a_\text{max}$
with $a < a_\text{max} +$ `zero`. Default is $(1/2)^{-50} \approx 9\times 10^{-16}$ .






--------------

# Mesh-related variables

##  cells

Number of cells of the unstructured grid. This number is the actual
quantity of volumetric elements in which the domain was discretized.



##  elements

Number of total elements of the unstructured grid. This number
include those surface elements that belong to boundary physical entities.



##  nodes

Number of nodes of the unstructured grid.






--------------

# Functions

##  abs

Returns the absolute value of the argument $x$.

~~~wasora
y = abs(x)
~~~



##  acos

Computes arc in radians whose cosine is equal to the argument $x$.
A NaN error is raised if $|x|>1$.

~~~wasora
y = acos(x)
~~~



##  asin

Computes arc in radians whose sine is equal to the argument $x$.
A NaN error is raised if $|x|>1$.

~~~wasora
y = asin(x)
~~~



##  atan

Computes, in radians, the arc tangent of the argument $x$.

~~~wasora
atan(x)
~~~



##  atan2

Computes, in radians, the arc tangent of quotient $y/x$, using the signs of the two arguments
to determine the quadrant of the result, which is in the range $[-\pi,\pi]$.

~~~wasora
atan(y,x)
~~~



##  ceil

Returns the smallest integral value not less than the argument $x$.

~~~wasora
ceil(x)
~~~



##  clock

Returns the value of a certain clock in seconds measured from a certain (but specific)
milestone. The kind of clock and the initial milestone depends on the
optional flag $f$. It defaults to zero, meaning wall time since the UNIX Epoch.
The list and the meanings of the other available values for $f$ can be checked
in the `clock_gettime (2)` system call manual page.

~~~wasora
clock([f])
~~~



##  cos

Computes the cosine of the argument $x$, where $x$ is in radians.
A cosine wave can be generated by passing as the argument $x$
a linear function of time such as $\omega t+\phi$, where $\omega$ controls the frequency of the wave
and $\phi$ controls its phase.

~~~wasora
cos(x)
~~~



##  cosh

Computes the hyperbolic cosine of the argument $x$, where $x$ is in radians.

~~~wasora
cosh(x)
~~~



##  d_dt

Computes the time derivative of the signal $x$ using the difference between the
value of the signal in the previous time step and the actual value divided by the
time step. For $t=0$, the return value is zero.
Unlike the functional `derivative`, this function works with expressions and not
with functions. Therefore the argument $x$ may be for example an expression
involving a variable that may be read from a shared-memory object, whose
time derivative cannot be computed with `derivative`.

~~~wasora
d_dt(x)
~~~



##  deadband

Filters the first argument $x$ with a deadband centered at zero with an amplitude
given by the second argument $a$.

~~~wasora
deadband(x, a)
~~~



##  equal

Checks if the two first expressions $a$ and $b$ are equal, up to the tolerance
given by the third optional argument $\epsilon$. If either $|a|>1$ or $|b|>1$,
the arguments are compared using GSL's `gsl_fcmp`, otherwise the
absolute value of their difference is compared against $\epsilon$. This function
returns \textsl{exactly} zero if the arguments are not equal and one otherwise.
Default value for $\epsilon = 10^{-16}$.

~~~wasora
equal(a, b, [eps])
~~~



##  exp

Computes the exponential function the argument $x$, i.e. the base of the
natural logarithms raised to the $x$-th power.

~~~wasora
exp(x)
~~~



##  expint1

Computes the first exponential integral function of the argument $x$.
If $x$ equals zero, a NaN error is issued.

~~~wasora
expint1(x)
~~~



##  expint2

Computes the second exponential integral function of the argument $x$.

~~~wasora
expint2(x)
~~~



##  expint3

Computes the third exponential integral function of the argument $x$.

~~~wasora
expint3(x)
~~~



##  expintn

Computes the $n$-th exponential integral function of the argument $x$.
If $n$ equals zero or one and $x$ zero, a NaN error is issued.

~~~wasora
expintn(n,x)
~~~



##  floor

Returns the largest integral value not greater than the argument $x$.

~~~wasora
floor(x)
~~~



##  heaviside

Computes the zero-centered Heaviside step function of the argument $x$.
If the optional second argument $\epsilon$ is provided, the discontinuous
step at $x=0$ is replaced by a ramp starting at $x=0$ and finishing at $x=\epsilon$.

~~~wasora
heaviside(x, [eps])
~~~



##  if

Performs a conditional testing of the first argument $a$, and returns either the
second optional argument $b$ if $a$ is different from zero or the third optional argument $c$
if $a$ evaluates to zero. The comparison of the condition $a$ with zero is performed
within the precision given by the optional fourth argument $\epsilon$.
If the second argument $c$ is not given and $a$ is not zero, the function returns one.
If the third argument $c$ is not given and $a$ is zero, the function returns zero.
The default precision is $\epsilon = 10^{-16}$.
Even though `if` is a logical operation, all the arguments and the returned value
are double-precision floating point numbers.

~~~wasora
if(a, [b], [c], [eps])
~~~



##  integral_dt

Computes the time integral of the signal $x$ using the trapezoidal rule
using the value of the signal in the previous time step and the current value.
At $t = 0$ the integral is initialized to zero.
Unlike the functional `integral`, this function works with expressions and not
with functions. Therefore the argument $x$ may be for example an expression
involving a variable that may be read from a shared-memory object, whose
time integral cannot be computed with `integral`.

~~~wasora
integral_dt(x)
~~~



##  integral_euler_dt

Idem as `integral_dt` but uses the backward Euler rule to update the
integral value.
This function is provided in case this particular way
of approximating time integrals is needed.

~~~wasora
integral_euler_dt(x)
~~~



##  is_even

Returns one if the argument $x$ rounded to the nearest integer is even.

~~~wasora
y = is_even(x)
~~~



##  is_in_interval

Returns true if the argument~$x$ is in the interval~$[a,b)$, i.e. including~$a$
but excluding~$b$.

~~~wasora
is_in_interval(x, a, b)
~~~



##  is_odd

Returns one if the argument $x$ rounded to the nearest integer is odd.

~~~wasora
y = is_odd(x)
~~~



##  j0

Computes the regular cylindrical Bessel function of zeroth order evaluated at the argument $x$.

~~~wasora
j0(x)
~~~



##  lag

Filters the first argument $x(t)$ with a first-order lag of characteristic time $\tau$,
i.e. this function applies the transfer function
!bt
\[ G(s) = \frac{1}{1 + s\tau} \]
!et
to the time-dependent signal $x(t)$, by assuming that it is constant during the time interval
$[t-\Delta t,t]$ and using the analytical solution of the differential equation for that case
at $t = \Delta t$ with the initial condition $y(0) = y(t-\Delta t)$.

~~~wasora
lag(x, tau)
~~~



##  lag_bilinear

Filters the first argument $x(t)$ with a first-order lag of characteristic time $\tau$,
i.e. this function applies the transfer function
!bt
\[ G(s) = \frac{1}{1 + s\tau} \]
!et
to the time-dependent signal $x(t)$ by using the bilinear transformation formula.

~~~wasora
lag_bilinear(x, tau)
~~~



##  lag_euler

Filters the first argument $x(t)$ with a first-order lag of characteristic time $\tau$,
i.e. this function applies the transfer function
!bt
\[ G(s) = \frac{1}{1 + s\tau} \]
!et
to the time-dependent signal $x(t)$ by using the Euler forward rule.

~~~wasora
lag_euler(x, tau)
~~~



##  last

Returns the value the signal $x$ had in the previous time step.
This function is equivalent to the $Z$-transform operator "delay" denoted by $z^{-1}\left[x\right]$.
For $t=0$ the function returns the actual value of $x$.
The optional flag $p$ should be set to one if the reference to `last`
is done in an assignment over a variable that already appears insi
expression $x$. See example number 2.

~~~wasora
last(x,[p])
~~~



##  limit

Limits the first argument $x$ to the interval $[a,b]$. The second argument $a$ should
be less than the third argument $b$.

~~~wasora
limit(x, a, b)
~~~



##  limit_dt

Limits the value of the first argument $x(t)$ so to that its time derivative
is bounded to the interval $[a,b]$. The second argument $a$ should
be less than the third argument $b$.

~~~wasora
limit_dt(x, a, b)
~~~



##  log

Computes the natural logarithm of the argument $x$. If $x$ is zero or negative,
a NaN error is issued.

~~~wasora
log(x)
~~~



##  mark_max

Returns the integer index $i$ of the maximum of the arguments $x_i$ provided. Currently only maximum of ten arguments can be provided.

~~~wasora
mark_max(x1, x2, [...], [x10])
~~~



##  mark_min

Returns the integer index $i$ of the minimum of the arguments $x_i$ provided. Currently only maximum of ten arguments can be provided.

~~~wasora
mark_max(x1, x2, [...], [x10])
~~~



##  max

Returns the maximum of the arguments $x_i$ provided. Currently only maximum of ten arguments can be provided.

~~~wasora
max(x1, x2, [...], [x10])
~~~



##  min

Returns the minimum of the arguments $x_i$ provided. Currently only maximum of ten arguments can be provided.

~~~wasora
min(x1, x2, [...], [x10])
~~~



##  mod

Returns the remainder of the division between the first argument $a$ and the
second $b$. Both arguments may be non-integral.

~~~wasora
mod(a, b)
~~~



##  not

Returns one if the first argument $x$ is zero and zero otherwise.
The second optional argument $\epsilon$ gives the precision of the "zero"
evaluation. If not given, default is $\epsilon = 10^{-16}$.

~~~wasora
not(x, [eps])
~~~



##  random

Returns a random real number uniformly distributed between the first
real argument $x_1$ and the second one $x_2$.
If the third integer argument $s$ is given, it is used as the seed and thus
repetitive sequences can be obtained. If no seed is provided, the current time
(in seconds) plus the internal address of the expression is used. Therefore,
two successive calls to the function without seed (hopefully) do not give the same result.
This function uses a second-order multiple recursive generator described by
Knuth in Seminumerical Algorithms, 3rd Ed., Section 3.6.

~~~wasora
random(x1, x2, [s])
~~~



##  random_gauss

Returns a random real number with a Gaussian distribution with a mean
equal to the first argument $x_1$ and a standard deviation equatl to the second one $x_2$.
If the third integer argument $s$ is given, it is used as the seed and thus
repetitive sequences can be obtained. If no seed is provided, the current time
(in seconds) plus the internal address of the expression is used. Therefore,
two successive calls to the function without seed (hopefully) do not give the same result.
This function uses a second-order multiple recursive generator described by
Knuth in Seminumerical Algorithms, 3rd Ed., Section 3.6.

~~~wasora
random_gauss(x1, x2, [s])
~~~



##  round

Rounds the argument $x$ to the nearest integer. Halfway cases are rounded away from zero.

~~~wasora
round(x)
~~~



##  sawtooth_wave

Computes a sawtooth wave betwen zero and one with a period equal to one.
As with the sine wave, a sawtooh wave can be generated by passing as the argument $x$
a linear function of time such as $\omega t+\phi$, where $\omega$ controls the frequency of the wave
and $\phi$ controls its phase.

~~~wasora
sawtooth_wave(x)
~~~



##  sgn

Returns minus one, zero or plus one depending on the sign of the first argument $x$.
The second optional argument $\epsilon$ gives the precision of the "zero"
evaluation. If not given, default is $\epsilon = 10^{-16}$.

~~~wasora
sgn(x, [eps])
~~~



##  sin

Computes the sine of the argument $x$, where $x$ is in radians.
A sine wave can be generated by passing as the argument $x$
a linear function of time such as $\omega t+\phi$, where $\omega$ controls the frequency of the wave
and $\phi$ controls its phase.

~~~wasora
sin(x)
~~~



##  sinh

Computes the hyperbolic sine of the argument $x$, where $x$ is in radians.

~~~wasora
sinh(x)
~~~



##  sqrt

Computes the positive square root of the argument $x$. If $x$ is negative,
a NaN error is issued.

~~~wasora
sqrt(x)
~~~



##  square_wave

Computes a square function betwen zero and one with a period equal to one.
The output is one for $0 < x < 1/2$ and goes to zero for $1/2 < x < 1$.
As with the sine wave, a square wave can be generated by passing as the argument $x$
a linear function of time such as $\omega t+\phi$, where $\omega$ controls the frequency of the wave
and $\phi$ controls its phase.

~~~wasora
square_wave(x)
~~~



##  tan

Computes the tangent of the argument $x$, where $x$ is in radians.

~~~wasora
tan(x)
~~~



##  tanh

Computes the hyperbolic tangent of the argument $x$, where $x$ is in radians.

~~~wasora
tanh(x)
~~~



##  threshold_max

Returns one if the first argument $x$ is greater than the threshold given by
the second argument $a$, and \textit{exactly} zero otherwise. If the optional
third argument $b$ is provided, an hysteresis of width $b$ is needed in order
to reset the function value. Default is no hysteresis, i.e. $b=0$.

~~~wasora
threshold_max(x, a, [b])
~~~



##  threshold_min

Returns one if the first argument $x$ is less than the threshold given by
the second argument $a$, and \textit{exactly} zero otherwise. If the optional
third argument $b$ is provided, an hysteresis of width $b$ is needed in order
to reset the function value. Default is no hysteresis, i.e. $b=0$.

~~~wasora
threshold_min(x, a, [b])
~~~



##  triangular_wave

Computes a triangular wave betwen zero and one with a period equal to one.
As with the sine wave, a triangular wave can be generated by passing as the argument $x$
a linear function of time such as $\omega t+\phi$, where $\omega $ controls the frequency of the wave
and $\phi$ controls its phase.

~~~wasora
triangular_wave(x)
~~~






--------------

# Functionals

##  derivative

Computes the derivative of the expression $f(x)$ 
given in the first argument with respect to the variable $x$
given in the second argument at the point $x=a$ given in
the third argument using an adaptive scheme.
The fourth optional argument $h$ is the initial width
of the range the adaptive derivation method starts with. 
The fifth optional argument $p$ is a flag that indicates
whether a backward ($p < 0$), centered ($p = 0$) or forward ($p > 0$)
stencil is to be used.
This functional calls the GSL functions
`gsl_deriv_central` or `gsl_deriv_forward`
according to the indicated flag $p$.
Defaults are $h = (1/2)^{-10} \approx 9.8 \times 10^{-4}$ and $p = 0$.

~~~wasora
derivative(f(x), x, a, [h], [p])
~~~



##  func_min

Finds the value of the variable $x$ given in the second argument
which makes the expression $f(x)$ given in the first argument to
take local a minimum in the in the range $[a,b]$ given by
the third and fourth arguments. If there are many local minima,
the one that is closest to $(a+b)/2$ is returned.
The optional fifth argument $\epsilon$ gives a relative tolerance
for testing convergence, corresponding to GSL `epsrel` (note that
`epsabs` is set also to $\epsilon)$.
The sixth optional argument is an integer which indicates the
algorithm to use:
0 (default) is `quad_golden`,
1 is `brent` and
2 is `goldensection`.
See the GSL documentation for further information on the algorithms.
The seventh optional argument $p$ is a flag that indicates how to proceed
if there is no local minimum in the range $[a,b]$.
If $p = 0$ (default), $a$ is returned if $f(a) < f(b)$ and $b$ otherwise.
If $p = 1$ then the local minimum algorimth is tried nevertheless.
Default is $\epsilon = (1/2)^{-20} \approx 9.6\times 10^{-7}$.

~~~wasora
y = func_min(f(x), x, a, b, [eps], [alg], [p])
~~~



##  gauss_kronrod

Computes the integral of the expression $f(x)$
given in the first argument with respect to variable $x$
given in the second argument over the interval $[a,b]$ given
in the third and fourth arguments respectively using a
non-adaptive procedure which uses fixed Gauss-Kronrod-Patterson
abscissae to sample the integrand at a maximum of 87 points.
It is provided for fast integration of smooth functions.
The algorithm applies the Gauss-Kronrod 10-point, 21-point,
43-point and 87-point integration rules in succession until an
estimate of the integral is achieved within the relative tolerance
given in the fifth optional argument $\epsilon$
It correspondes to GSL's `epsrel` parameter (`epsabs` is set to zero).  
The rules are designed in such a way that each rule uses all
the results of its predecessors, in order to minimize the total
number of function evaluations.
Defaults are $\epsilon = (1/2)^{-10} \approx 10^{-3}$.
See GSL reference for further information.

~~~wasora
gauss_kronrod(f(x), x, a, b, [eps])
~~~



##  gauss_legendre

Computes the integral of the expression $f(x)$
given in the first argument with respect to variable $x$
given in the second argument over the interval $[a,b]$ given
in the third and fourth arguments respectively using the
$n$-point Gauss-Legendre rule, where $n$ is given in the
optional fourth argument. 
It is provided for fast integration of smooth functions with
known polynomic order (it is exact for polynomials of order
$2n-1$).
This functional calls GSL function `gsl_integration_glfixedp`.
Default is $n = 12$.
See GSL reference for further information.

~~~wasora
gauss_legendre(f(x), x, a, b, [n])
~~~



##  integral

Computes the integral of the expression $f(x)$
given in the first argument with respect to variable $x$
given in the second argument over the interval $[a,b]$ given
in the third and fourth arguments respectively using an adaptive
scheme, in which the domain is divided into a number of maximum number
of subintervals
and a fixed-point Gauss-Kronrod-Patterson scheme is applied to each
quadrature subinterval. Based on an estimation of the error commited,
one or more of these subintervals may be split to repeat
the numerical integration alogorithm with a refined division.
The fifth optional argument $\epsilon$ is is a relative tolerance
used to check for convergence. It correspondes to GSL's `epsrel` 
parameter (`epsabs` is set to zero). 
The sixth optional argument $1\leq k \le 6$ is an integer key that
indicates the integration rule to apply in each interval.
It corresponds to GSL's parameter `key`. 
The seventh optional argument gives the maximum number of subdivisions,
which defaults to 1024.
If the integration interval $[a,b]$ if finite, this functional calls
the GSL function `gsl_integration_qag`. If $a$ is less that minus the
internal variable `infinite`, $b$ is greater that `infinite` or both
conditions hold, GSL functions `gsl_integration_qagil`, 
`gsl_integration_qagiu` or `gsl_integration_qagi` are called.
The condition of finiteness of a fixed range $[a,b]$ can thus be 
changed by modifying the internal variable `infinite`.
Defaults are $\epsilon = (1/2)^{-10} \approx 10^{-3}$ and $k=3$.
The maximum numbers of subintervals is limited to 1024.
Due to the adaptivity nature of the integration method, this function
gives good results with arbitrary integrands, even for
infinite and semi-infinite integration ranges. However, for certain
integrands, the adaptive algorithm may be too expensive or even fail
to converge. In these cases, non-adaptive quadrature functionals ought to
be used instead.
See GSL reference for further information.

~~~wasora
integral(f(x), x, a, b, [eps], [k], [max_subdivisions])
~~~



##  prod

Computes product of the $N=b-a$ expressions $f(i)$
given in the first argument by varying the variable~$i$
given in the second argument between~$a$
given in the third argument and~$b$
given in the fourth argument,~$i = a, a+1, \dots ,b-1,b$. 

~~~wasora
prod(f(i), i, a, b)
~~~



##  root

Computes the value of the variable $x$ given in the second argument
which makes the expression $f(x)$ given in the first argument to
be equal to zero by using a root bracketing algorithm.
The root should be in the range $[a,b]$ given by the third and fourth arguments.
The optional fifth argument $\epsilon$ gives a relative tolerance
for testing convergence, corresponding to GSL `epsrel` (note that
`epsabs` is set also to $\epsilon)$.
The sixth optional argument is an integer which indicates the
algorithm to use:
0 (default) is `brent`,
1 is `falsepos` and
2 is `bisection`.
See the GSL documentation for further information on the algorithms.
The seventh optional argument $p$ is a flag that indicates how to proceed
if the sign of $f(a)$ is equal to the sign of $f(b)$.
If $p=0$ (default) an error is raised, otherwise it is not.
If more than one root is contained in the specified range, the first
one to be found is returned. The initial guess is $x_0 = (a+b)/2$.
If no roots are contained in the range and
$p \neq 0$, the returned value can be any value.
Default is $\epsilon = (1/2)^{-10} \approx 10^{3}$.

~~~wasora
root(f(x), x, a, b, [eps], [alg], [p])
~~~



##  sum

Computes sum of the $N=b-a$ expressions $f_i$
given in the first argument by varying the variable $i$
given in the second argument between $a$
given in the third argument and $b$
given in the fourth argument, $i=a,a+1,\dots,b-1,b$. 

~~~wasora
sum(f_i, i, a, b)
~~~






--------------

# Vector functions

##  vecdot

Computes the dot product between vectors $\vec{a}$ and $\vec{b}$, which should
have the same size.

~~~wasora
vecdot(a,b)
~~~



##  vecmax

Returns the biggest element of vector $\vec{b}$, taking into account its sign
(i.e. $1 > -2$).

~~~wasora
vecmax(b)
~~~



##  vecmaxindex

Returns the index of the biggest element of vector $\vec{b}$, taking into account its sign
(i.e. $2 > -1$).

~~~wasora
vecmaxindex(b)
~~~



##  vecmin

Returns the smallest element of vector $\vec{b}$, taking into account its sign
(i.e. $-2 < 1$).

~~~wasora
vecmin(b)
~~~



##  vecminindex

Returns the index of the smallest element of vector $\vec{b}$, taking into account its sign
(i.e. $-2 < 1$).

~~~wasora
vecminindex(b)
~~~



##  vecnorm

Computes euclidean norm of vector $\vec{b}$. Other norms can be computed explicitly
using the `sum` functional, as illustrated in the example.

~~~wasora
vecnorm(b)
~~~



##  vecsize

Returns the size of vector $\vec{b}$.

~~~wasora
vecsize(b)
~~~



##  vecsum

Computes the sum of all the components of vector $\vec{b}$.

~~~wasora
vecsum(b)
~~~






--------------
