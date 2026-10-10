#!/bin/bash

num_nodes=$(cat outdoor-group-mobility-time-series.mob | awk '{ print $2 }' | sort -n | uniq | wc -l)
if [ "$num_nodes" -ne "3" ]; then
	echo "Exiting: this tracing program designed only for 3 nodes"
	exit 1
fi
cat outdoor-group-mobility-time-series.mob | awk -F " " '{ print $3 }' | awk -F ":" '{ print $1" "$2 }' >ogm-time-series.tmp
n=0
while read p1 && read p2 && read p3; do
	basename=$(printf "%04d" "$n")
	cat >plotcmds <<EOL
# If you do not have pngcairo installed, you can 'set terminal png' below,
# but it will render the dashed bounding box as a solid line
set terminal pngcairo dashed
set output '$basename.png'
set view map
set xlabel 'X [m]'
set ylabel 'Y [m]'
set xrange [-10:110]
set yrange [-10:60]
set style fill transparent solid 0.5
unset key
set style fill  transparent solid 0.35 noborder
set style circle radius 0.5
# Define dashed lines to mark the outer bounding box
set arrow from 0,0 to 100,0 nohead dt "-" lc rgb "light-grey"
set arrow from 100,0 to 100,50 nohead dt "-" lc rgb "light-grey"
set arrow from 0,50 to 100,50 nohead dt "-" lc rgb "light-grey"
set arrow from 0,0 to 0,50 nohead dt "-" lc rgb "light-grey"
plot "<echo '$p1'" with circles lc rgb "red",\
     "<echo '$p2'" with circles lc rgb "blue",\
     "<echo '$p3'" with circles lc rgb "black"
EOL
	gnuplot outdoor-group-mobility-buildings.txt plotcmds
	rm plotcmds
	((n++))
done <ogm-time-series.tmp
rm ogm-time-series.tmp
