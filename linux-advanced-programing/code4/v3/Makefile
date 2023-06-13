v3: 
	g++ -o main main.cpp CPluginEnumerator.cpp CPluginController.cpp -ldl
	g++ -fpic -shared -o plugin/libfunc.so function.cpp
	g++ -fpic -shared -o plugin/libfunc1.so function1.cpp
main:
	g++ -o main main.cpp CPluginEnumerator.cpp CPluginController.cpp -ldl
libfunc:
	g++ -fpic -shared -o plugin/libfunc.so function.cpp
	g++ -fpic -shared -o plugin/libfunc1.so function1.cpp
