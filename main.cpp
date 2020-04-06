#include "csv2h5.hpp"
#include <chrono>
#include <iostream>

int main(int argc, char* argv[]){
	
	std::vector<std::string> myArgs;
	for(int i = 1; i < argc; i++)
		myArgs.push_back(argv[i]);

	if (argc < 4){
		std::cerr << "Usage : " << argv[0] << " <input CSV> <output H5> <GroundTruth column> <fraction in initial timestamp (default: 1)> <instances per evolved timestamp (default: 0)>" << std::endl;
		}
	else if (argc == 4)
		csv2h5(myArgs[0], myArgs[1], myArgs[2]);	
	else if (argc == 5)
		csv2h5(myArgs[0], myArgs[1], myArgs[2], std::stof(myArgs[3]));	
	else if (argc == 6) 
		csv2h5(myArgs[0], myArgs[1], myArgs[2], std::stof(myArgs[3]), std::stof(myArgs[4]));	

}
