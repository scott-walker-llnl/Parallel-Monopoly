sequential: mono.c
	gcc mono.c -o seq.out

parallel: par.c
	mpicc par.c -o par.out

clean:
	rm *.out
	rm *.dbg
