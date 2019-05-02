set ticslevel 0

set size square
unset colorbox

set xtics 1
set ytics 1
set xlabel "x"
set ylabel "y"
set zlabel "z"

set terminal svg
set output "scattered1a.svg"

set view 65,250

splot \
      "n_int.dat" u 1:2:3 w p pt 57 ps 0.25 palette ti "nearest",\
      "n_def.dat" w p ps 1.5 lt 3 pt 2 ti "definition points"


set terminal svg
set output "scattered1b.svg"

set view 45,195
replot




set terminal svg
set output "scattered1c.svg"

set view 55,100
replot



set terminal svg
set output "scattered2d1.svg"

set xrange [-1:1]
set yrange [-1:1]
plot "n_int.dat" u 1:2:3 w image ti "",\
     "n_def.dat" w p ps 1.5 lt 3 pt 2 ti ""




# ----------------------------------
set terminal svg
set output "scattered2a.svg"

set view 65,250

splot \
      "n_int.dat" u 1:2:4 w p pt 57 ps 0.25 palette ti "shepard",\
      "n_def.dat" w p ps 1.5 lt 3 pt 2 ti "definition points"




set terminal svg
set output "scattered2b.svg"

set view 45,195
replot




set terminal svg
set output "scattered2c.svg"

set view 55,100
replot



set terminal svg
set output "scattered2d2.svg"

set xrange [-1:1]
set yrange [-1:1]
plot "n_int.dat" u 1:2:4 w image ti "",\
     "n_def.dat" w p ps 1.5 lt 3 pt 2 ti ""


# ----------------------------------
set terminal svg
set output "scattered3a.svg"

set view 65,250

splot \
      "n_int.dat" u 1:2:5 w p pt 57 ps 0.25 palette ti "modified shepard",\
      "n_def.dat" w p ps 1.5 lt 3 pt 2 ti "definition points"




set terminal svg
set output "scattered3b.svg"

set view 45,195
replot




set terminal svg
set output "scattered3c.svg"

set view 55,100
replot


set terminal svg
set output "scattered2d3.svg"

set xrange [-1:1]
set yrange [-1:1]
plot "n_int.dat" u 1:2:5 w image ti "",\
     "n_def.dat" w p ps 1.5 lt 3 pt 2 ti ""

