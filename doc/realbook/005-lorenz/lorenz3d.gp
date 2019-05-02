set ticslevel 0
set xlabel "x"
set ylabel "y"
set zlabel "z"
set xtics 10
set ytics 10
set ztics 10
unset key
# set terminal pdf size 14cm,9cm
# set output "lorenz3d.pdf"

set terminal svg
set output "lorenz3d.svg"

splot "lorenz.dat" u 2:3:4 w l lt 3

# set terminal png size 800,600
# set output "lorenz3d.png"

# replot
# 