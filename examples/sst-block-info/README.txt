mpirun -np 4 ./writer SST foo

mpirun -np 1 ./reader SST foo
