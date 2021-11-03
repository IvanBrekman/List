cr:
	clear
	gcc main.cpp libs/baselib.cpp libs/file_funcs.cpp list.cpp -o main.out
	./main.out

c:
	gcc main.cpp libs/baselib.cpp libs/file_funcs.cpp list.cpp -o main.out

r:
	./main.out
