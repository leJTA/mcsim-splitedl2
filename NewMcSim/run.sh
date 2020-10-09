#/bin/bash 

tmp="120807"
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
done
