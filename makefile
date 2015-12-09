sequential: mono.c
	gcc -O3 mono.c -o seq.out

parallel: par.c
	mpicc -O3 par.c -o par.out

clean:
	rm *.out
	rm *.dbg
