test : Suber.o CSuber.o
	g++ -o test Suber.o CSuber.o
	rm *.o
Suber.o : Suber.cpp CSuber.h
	g++ -o Suber.o -c Suber.cpp
CSuber.o : CSuber.cpp CSuber.h
	g++ -o CSuber.o -c CSuber.cpp