
csv2h5 : main.o csv2h5.o
	g++ -O3 main.o csv2h5.o -lz -ldl -lm -L/usr/local/hdf5/lib -lhdf5_cpp -lhdf5 -o csv2h5

csv2h5.o : csv2h5.hpp csv2h5.cpp
	g++ -O3 -I/usr/local/hdf5/include -c csv2h5.cpp csv2h5.hpp

main.o : main.cpp csv2h5.hpp
	g++ -O3 -I/usr/local/hdf5/include -c main.cpp csv2h5.hpp

clean :
	rm *.o *.gch 
	rm *.h5
