#!/bin/bash

mkdir -p outdoor-random-walk-animation
cat mobility-trace-example.mob | awk -F " " '{ print $3 }' | awk -F "=" '{ print $2 }' | awk -F ":" '{ print $1" "$2 }' >mobility-trace-reduced.txt
n=0
while read p; do
	basename=$(printf "%04d" "$n")
	cat >plotcmds <<EOL
set terminal png
set output '$basename.png'
set view map
set xlabel 'X [m]'
set ylabel 'Y [m]'
set xrange [-25:1300]
set yrange [-25:800]
set style fill transparent solid 0.5
unset key
set style fill  transparent solid 0.35 noborder
set style circle radius 5
plot "<echo '$p'" with circles lc rgb "blue"
EOL
	gnuplot buildings.txt plotcmds
	rm plotcmds
	mv $basename.png outdoor-random-walk-animation
	((n++))
done <mobility-trace-reduced.txt
rm mobility-trace-reduced.txt
