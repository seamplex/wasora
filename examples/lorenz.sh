wasora lorenz.was > lorenz.dat
head lorenz.dat
tail lorenz.dat
gnuplot -e "set terminal svg; set output 'lorenz.svg'; set ticslevel 0; splot 'lorenz.dat' u 2:3:4 w l ti ''"
