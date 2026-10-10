#!/bin/bash

control_c() {
	echo "Aborted, exiting..."
	exit $?
}
trap control_c SIGINT

scriptDir=$(pwd)
cd ../../../
ns3Dir=$(pwd)
cd $scriptDir

if test ! -f ${ns3Dir}/ns3; then
	echo "Please run this script from within the directory $(dirname $0), like this:"
	echo "cd $(dirname $0)"
	echo "./$(basename $0)"
	exit 1
fi

outputDir=$(pwd)/wifi-trans-results
if [ -d $outputDir ]; then
	echo "$outputDir directory exists."
else
	mkdir -p "$outputDir"
	echo "$outputDir directory created."
fi

std_leg=("11a" "11_10MHZ" "11_5MHZ")
std_n=("11n_2_4GHZ" "11n_5GHZ")
std_ac_ax=("11ac" "11ax_2_4GHZ" "11ax_5GHZ")
bw_leg=(20 10 5)
bw_n=(20 40)
bw_ac_ax=(20 40 80 160)

pre="spectrum-analyzer-wifi-"
suf="-2-0"

for i in 0 1 2; do
	std=${std_leg[${i}]}
	bw=${bw_leg[${i}]}
	echo "==============================================="
	echo "Run for wifi-trans-example for ${std} and ${bw} MHz"
	cd $ns3Dir
	./ns3 run "wifi-trans-example --standard=$std --bw=$bw"
	echo "Generate PSD using ${file}.tr"
	file="${pre}${std}-${bw}MHz${suf}"
	gnuplot ${file}.plt
	if test ! -f ${file}.png; then
		echo "PNG file (${file}.png) was not generated, something went wrong!"
		exit 1
	else
		echo "PNG file (${file}.png) generated, remove .tr and .plt files"
		mv ${file}.png $outputDir
		rm -f ${file}.*
		echo ""
	fi
done

for std in "${std_n[@]}"; do
	for bw in "${bw_n[@]}"; do
		echo "==============================================="
		echo "Run for wifi-trans-example for ${std} and ${bw} MHz"
		cd $ns3Dir
		./ns3 run "wifi-trans-example --standard=$std --bw=$bw"
		echo "Generate PSD using ${file}.tr"
		file="${pre}${std}-${bw}MHz${suf}"
		gnuplot ${file}.plt
		if test ! -f ${file}.png; then
			echo "PNG file (${file}.png) was not generated, something went wrong!"
			exit 1
		else
			echo "PNG file (${file}.png) generated, remove .tr and .plt files"
			mv ${file}.png $outputDir
			rm -f ${file}.*
			echo ""
		fi
	done
done

for std in "${std_ac_ax[@]}"; do
	for bw in "${bw_ac_ax[@]}"; do
		[ ${std} = "11ax_2_4GHZ" ] && ([ ${bw} = "80" ] || [ ${bw} = "160" ]) && continue
		echo "==============================================="
		echo "Run for wifi-trans-example for ${std} and ${bw} MHz"
		cd $ns3Dir
		./ns3 run "wifi-trans-example --standard=$std --bw=$bw"
		echo "Generate PSD using ${file}.tr"
		file="${pre}${std}-${bw}MHz${suf}"
		gnuplot ${file}.plt
		if test ! -f ${file}.png; then
			echo "PNG file (${file}.png) was not generated, something went wrong!"
			exit 1
		else
			echo "PNG file (${file}.png) generated, remove .tr and .plt files"
			mv ${file}.png $outputDir
			rm -f ${file}.*
			echo ""
		fi
	done
done

echo "Finished"
