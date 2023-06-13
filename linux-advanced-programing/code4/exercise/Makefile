exercise: 
	g++ -o main main.cpp CPluginEnumerator.cpp CPluginController.cpp IPrintPlugin.cpp -ldl
	g++ -fpic -shared -o plugin/libfunc.so function.cpp IPrintPlugin.cpp
	g++ -fpic -shared -o plugin/libfunc1.so function1.cpp IPrintPlugin.cpp
main:
	g++ -o main main.cpp CPluginEnumerator.cpp CPluginController.cpp IPrintPlugin.cpp -ldl
libfunc:
	g++ -fpic -shared -o plugin/libfunc.so function.cpp IPrintPlugin.cpp
	g++ -fpic -shared -o plugin/libfunc1.so function1.cpp IPrintPlugin.cpp