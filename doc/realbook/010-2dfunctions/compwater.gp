set terminal pdf size 5,2.8
set output "compwater.pdf"
set ticslevel 0
set tics scale 0.2
unset colorbox
set xlabel "pressure [Pa]"
set ylabel "temperature [K]"
set zlabel "enthalphy [j/kg]"

set xrange [0:200e5]
set yrange [300:800]

set xtics 5e6
set ytics 250
set ztics 1e6

unset key

#set view 70, 310
splot "compwater.dat" w l palette, "compwater.txt" u 2:1:($5+$2*$4) pt 5 ps 0.5 palette

set terminal svg
set output "compwater.svg"
replot
