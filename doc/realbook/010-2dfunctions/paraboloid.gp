set hidden3d
#set pm3d
set ticslevel 0

set size square
unset colorbox

set xtics 1
set ytics 1
set xlabel "x"
set ylabel "y"
set zlabel "f(x,y)"

set terminal svg
set output "paraboloid.svg"

splot "paraboloid.dat" w pm3d ti ""

# set terminal png
# set output "paraboloid.png"
# replot

set terminal svg
set output "paraboloid2d.svg"

set xrange [-2:2]
set yrange [-2:2]
plot "paraboloid.dat" w image

# set terminal png
# set output "paraboloid2d.png"
# replot
