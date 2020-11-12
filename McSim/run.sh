#/bin/bash

tmp="154963"
echo -n "" > output/results.csv
for str in 33779 245 24708 3941 135030 25578 131982 154963 
do
   echo "==================================================================="
   echo "[Simulating $str]"
   sed -i "s/$tmp.trace/$str.trace/g" ../Apps/list/run-trace.py
   echo "Command : ./launch_mcsim.sh trace $1 | grep "\[000\]" > output/$str.out"
   ./launch_mcsim.sh trace $1 | grep "\[000\]" > output/$str.out
   echo "[$str simulated]"
   echo "==================================================================="
   tmp=$str

   cat output/$str.out | grep "L1\$I\[000\] : RD (miss, access)" | awk -F "[()]" '{print $4}' | awk -F "," '{print $2 "," $1 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L1I RD
   cat output/$str.out | grep "L1\$D\[000\] : RD (miss, access)" | awk -F "[()]" '{print $4}' | awk -F "," '{print $2 "," $1 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L1D RD
   cat output/$str.out | grep "L2\$ \[000\] : RD (miss, access)" | awk -F "[()]" '{print $4}' | awk -F "," '{print $2 "," $1 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L2 RD
   cat output/$str.out | grep "L2\$ \[000\] : WR (miss, access)" | awk -F "[()]" '{print $4}' | awk -F "," '{print $2 "," $1}' | xargs -I {} -t echo -n {} >> output/results.csv #L2 WR
   for num in 0 1 2 3 4 5 6 7
   do
      cat output/$str.out | grep "L2\$ \[000\] : RD miss from L1\$i \[00$num\]" | awk -F ":" '{print $3 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L2 miss read from L1i[i]
      cat output/$str.out | grep "L2\$ \[000\] : RD miss from L1\$d \[00$num\]" | awk -F ":" '{print $3 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L2 miss read from L1d[i]
      cat output/$str.out | grep "L2\$ \[000\] : WR miss from L1\$d \[00$num\]" | awk -F ":" '{print $3 ","}' | xargs -I {} -t echo -n {} >> output/results.csv #L2 miss write from L1d[i]
   done
   printf "\n" >> output/results.csv
done
