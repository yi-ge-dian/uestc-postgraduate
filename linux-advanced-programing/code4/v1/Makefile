v1: 
	g++ -fpic -shared -o libfunc.so function.cpp 
	g++ -o main main.cpp -ldl

libfunc:
	g++ -fpic -shared -o libfunc.so function.cpp 

main: 
	g++ -o main main.cpp -ldl