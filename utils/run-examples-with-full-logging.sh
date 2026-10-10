#!/bin/bash

cd ..
$(./test.py -l >&/tmp/test.out)

while read line; do
	if [[ "$line" == example* ]]; then
		name=${line#example      }
		NS_LOG="*" ./ns3 --run "$name" >&/dev/null
		status="$?"
		echo "program $name status $status"
	fi
done <"/tmp/test.out"

rm -rf /tmp/test.out
cd utils
