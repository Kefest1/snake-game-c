make main.c:
	gcc main.c snake.c snake_list.c -o main.out -lncurses -pthread -lrt

