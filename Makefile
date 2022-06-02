all: prsim test 
prsim: prsim.c
		gcc -o prsim prsim.c


test: test.c
		gcc -o test -Wall test.c

clean: 
		rm prsim test


# prsim: prsim.c
# gcc -o prsim prsim.c -I