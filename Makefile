.PHONY: clean

LIB_SO=libhf.so
OPT=-g

%.o: %.c 
	# -Wextra
	# -Werror
	gcc $(OPT) -Wall -std=c99 -fPIC -c $< -o $@

$(LIB_SO): huff.o
	gcc -shared -fPIC -o $@ $^

test: $(LIB_SO) main.c
	gcc $(OPT) main.c -o $@ -Wl,-rpath=. -L. -lhf

clean:
	-rm $(LIB_SO)
	-rm test