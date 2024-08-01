EVSimulation: EVSimulation.c
	mpicc EVSimulation.c -o EVSimulation_Out -fopenmp

run:
	mpirun -np ${n} -oversubscribe ./EVSimulation_Out ${x} ${y}

clean:
	/bin/rm -f EVSimulation_Out *.o