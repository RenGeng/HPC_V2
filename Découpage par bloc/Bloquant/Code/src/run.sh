make

mpirun -n 4 -bynode ./bin/shalw --export --export-path /tmp/

./visu.py /tmp/shalw_256x256_T1000.sav 

rm -f /tmp/shalw_256x256_T1000.sav