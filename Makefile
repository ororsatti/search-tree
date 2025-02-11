search: main.c dist
	gcc main.c -o dist/search -Wall -Wextra

dist:
	mkdir dist
