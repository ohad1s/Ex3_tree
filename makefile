
.PHONY: all
all: task

task:	 stree.c
			gcc -Wall -g -o stree stree.c

.PHONY: clean
clean:
	-rm stree