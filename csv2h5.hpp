#include <string>
#include <vector>
#include "H5Cpp.h"

struct 	pointProperties{
	std::string *csvLine;
	unsigned myTimeStamp;
	unsigned myPosition;
	float gtValue;
	float pointID;
	float *toWrite;
	std::vector<float> featVector;
};

struct 	H5Properties{
	std::string h5FileName;
	unsigned numInitInstances;	
	unsigned numBatchInstances;
	unsigned numLastBatchInstances;
	unsigned numSnapshots;
	float initPercent;
};
struct 	csvProperties{
	std::string csvFileName;
	std::string labelColumn;
	std::vector<std::string> FeatureNames;
	unsigned int numLines;
	unsigned int numFeatures;
	unsigned int gtCol;
};

void	csv2h5(std::string, std::string, std::string, float=1, unsigned=0);
void 	csvExtractProperties(csvProperties *);
void 	initializeH5(H5Properties *, csvProperties *);
void 	csv2h5Convert(H5Properties *, csvProperties *);
void 	initDataSet(std::string, H5::Group *, std::vector<int> &);
int  	pointsCurrTimeStamp(int, H5Properties *);
void 	parseLine(pointProperties *, csvProperties *);
void 	getDataPlacement(pointProperties *, H5Properties *);
void 	writeData(H5::H5File *, std::string, pointProperties *);
void 	writeFeatures(H5::H5File *, csvProperties *);
