#/bin/bash

tmp="120807"
echo -n "" > output/results.csv
for str in  186024 148294 199 120807
do 
echo "==================================================================="
echo "[Simulating $str]"
sed -i "s/$tmp/$str/g" ../Apps/list/run-trace.py
echo "Command : ./launch_mcsim.sh trace $1 | grep "\[000\]" > output/$str.out"
./launch_mcsim.sh trace $1 | grep "\[000\]" > output/$str.out
echo "[$str simulated]"
echo "==================================================================="
tmp=$str

cat output/$str.out | grep "L1\$I\[000\] : RD" | awk -F "[()]" '{print $4}' | awk -F "," '{print $2 "," $1 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L1I RD
cat output/$str.out | grep "L1\$D\[000\] : RD" | awk -F "[()]" '{print $4}' | awk -F "," '{print $2 "," $1 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L1D RD
cat output/$str.out | grep "L2\$ \[000\] : RD" | awk -F "[()]" '{print $4}' | awk -F "," '{print $2 "," $1 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L2 RD
cat output/$str.out | grep "L2\$ \[000\] : WR" | awk -F "[()]" '{print $4}' | awk -F "," '{print $2 "," $1}' | xargs -I {} -t echo -n {} >> output/results.csv #L2 WR
printf "\n" >> output/results.csv

done
