checklinks: main.o checklinks.o
	gcc -g -Wall -Wstrict-prototypes main.o checklinks.o -o checklinks
main.o: main.c checklinks.h
	gcc -g -Wall -Wstrict-prototypes -c main.c
checklinks.o: checklinks.c checklinks.h
	gcc -g -Wall -Wstrict-prototypes -c checklinks.c
clean:
	rm *.o checklinks

