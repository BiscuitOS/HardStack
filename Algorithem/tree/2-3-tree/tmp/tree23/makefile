objects = main.o tree23.o

mktree: $(objects)
	gcc -o mktree $(objects)
main.o: main.c
	gcc -c main.c
tree23.o: tree23.c
	gcc -c tree23.c
clean:
	rm $(objects) mktree
