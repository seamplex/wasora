set ticslevel 0

set size square
unset colorbox
set view 60,290

set xtics 1
set ytics 1
set xlabel "x"
set ylabel "y"
set zlabel "g(x,y)"

set terminal svg
set output "nearest.svg"

splot \
      "g_int.dat" w pm3d ti "",\
      "g_def.dat" w p ps 1.5 lt 3 pt 2 ti "definition points"

# set terminal png
# set output "nearest.png"
# 
# replot


set terminal svg
set output "nearest2d.svg"

set xrange [0:2]
set yrange [0:2]
plot "g_int.dat" w image ti "",\
     "g_def.dat" w p ps 1.5 lt 3 pt 2 ti ""

# set terminal png
# set output "nearest2d.png"
# 
# replot
