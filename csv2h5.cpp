#include "csv2h5.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

void csv2h5(std::string csvFN, std::string h5FN, std::string lc, float ip, unsigned sb){

	// Extract CSV File Properties
	csvProperties csvObj;
	csvObj.csvFileName = csvFN;	
	csvObj.labelColumn = lc;
	csvExtractProperties( &csvObj );

	// Create H5 File and initilize it
	H5Properties h5Obj;
	h5Obj.h5FileName = h5FN;
	h5Obj.initPercent = ip;
	h5Obj.numBatchInstances = sb;
	initializeH5( &h5Obj, &csvObj ); 

	// Decode CSV and transfer to H5 
	csv2h5Convert(&h5Obj, &csvObj);
}


void csvExtractProperties(csvProperties *ptr){
	std::string line, tmp;
	std::ifstream file(ptr->csvFileName);

	// Get the Feature List
	std::getline(file,line);
	std::stringstream sline(line);
	while(std::getline(sline, tmp, ',')){
		tmp.erase(std::remove(tmp.begin(),tmp.end(),'"'),tmp.end());
		ptr->FeatureNames.push_back(tmp);	
	}	
	// Set number of features
	ptr->numFeatures = ptr->FeatureNames.size() - 1;		

	// Get the number of lines 
	ptr->numLines = 0;
	while(std::getline(file, line))
		ptr->numLines++;

	// Close file
	file.close();

	// Find column number of the GT
	bool success = false;
	while(!success){
		std::vector<std::string>::iterator it;
		it = find(ptr->FeatureNames.begin(), ptr->FeatureNames.end(), ptr->labelColumn);
		unsigned dist = std::distance(ptr->FeatureNames.begin(),it);
		if( dist < ptr->FeatureNames.size() ){
			ptr->gtCol = dist;
			success = true;
		}else{
			std::cout << "Ground Truth column not found. " << std::endl; 
			std::cout << "Please re-introduce column name: ";
			std::cin >> ptr->labelColumn;
		}
	}

}

void initializeH5(H5Properties *ptr_h5, csvProperties *ptr_csv){
	
	// Correct if fraction is out of range
	if((ptr_h5->initPercent > 1)||(ptr_h5->initPercent < 0)){
		ptr_h5->initPercent = 1;
		std::cout << "Fraction must be between (0,1]. Setting fraction to 1..." << std::endl;
	}

	// Compute number of points initial time stamp
	ptr_h5->numInitInstances = ptr_csv->numLines*ptr_h5->initPercent;

	// Check if user set fraction smaller than 1 and batches equal to 0
	if((ptr_h5->initPercent < 1)&&(ptr_h5->numBatchInstances == 0)){
		ptr_h5->numBatchInstances = ptr_csv->numLines - ptr_h5->numInitInstances;
		std::cout << "Current version stores data. To use the desired fraction use Timestamp_0, the rest of data will be stored in Timestamp_1." << std::endl;
	}

	// Compute number of time stamps
	if(ptr_h5->numInitInstances == ptr_csv->numLines)
		ptr_h5->numSnapshots = 1;
	else{
		unsigned tmp1, tmp2;
		tmp1 = (ptr_csv->numLines - ptr_h5->numInitInstances)/ptr_h5->numBatchInstances;
		tmp2 = (ptr_csv->numLines - ptr_h5->numInitInstances)%ptr_h5->numBatchInstances;
		ptr_h5->numSnapshots = tmp2 == 0 ? tmp1+1 : tmp1+2;

		// Compute number of points in last time stamp
		ptr_h5->numLastBatchInstances = ptr_h5->numBatchInstances - ((ptr_h5->numSnapshots-1)*
		ptr_h5->numBatchInstances%(ptr_csv->numLines - ptr_h5->numInitInstances));
	}


	// Create H5 File
	H5::H5File file(ptr_h5->h5FileName, H5F_ACC_TRUNC);	

	// Setup the features as attributes
	writeFeatures(&file, ptr_csv);

	// Create one group per time stamp and initialize datasets
	for(int i = 0; i < ptr_h5->numSnapshots; i++){
		// Timestamp
		H5::Group gp(file.createGroup("Timestamp_" + std::to_string(i)));		

		// Number of instances for current timestamp
		int numPoints = pointsCurrTimeStamp(i, ptr_h5); 

		// Initialize Features Dataset
		std::vector<int> dims;
		dims.push_back( numPoints );
		dims.push_back( ptr_csv->numFeatures );
		initDataSet("Features", &gp, dims);

		// Initialize Ground Truth Dataset
		dims.clear();
		dims.push_back( numPoints );
		initDataSet("GroundTruth", &gp, dims);

		// Initialize IDs Dataset
		dims.clear();
		dims.push_back( numPoints );
		initDataSet("IDs", &gp, dims);

		// Initialize Prediction Dataset
		dims.clear();
		dims.push_back( numPoints );
		initDataSet("Prediction", &gp, dims);
	}

	// Close file
	file.close();
}

void csv2h5Convert(H5Properties *ptr_h5, csvProperties *ptr_csv){

	// Open csv and H5 Files
	std::ifstream csvFile(ptr_csv->csvFileName);
	H5::H5File h5File(ptr_h5->h5FileName, H5F_ACC_RDWR);
	
	// Skip First Line
	std::string line; std::getline(csvFile,line);

	// Read data point: Store its Features and its Class 	
	unsigned currPoint(0);
	while(std::getline(csvFile,line)){ 
		
		pointProperties dataPoint;
		dataPoint.pointID = currPoint;
		dataPoint.csvLine = &line;
		
		// Parse datapoint info
		parseLine(&dataPoint, ptr_csv);

		// Get correspondig timestamp and position in dataset of datapoint
		getDataPlacement(&dataPoint, ptr_h5);

		// Store Ground Truth Value
		dataPoint.toWrite = &dataPoint.gtValue;
		writeData(&h5File, "GroundTruth", &dataPoint);

		// Store ID Value
		dataPoint.toWrite = &dataPoint.pointID;
		writeData(&h5File, "IDs", &dataPoint);

		// Store Features
		dataPoint.toWrite = dataPoint.featVector.data();
		writeData(&h5File, "Features", &dataPoint);

		currPoint++;
	}

	// Close Files
	h5File.close();
	csvFile.close();
}

void initDataSet(std::string dataSetName, H5::Group *Obj, std::vector<int> &dims){
	hsize_t dataSetDims[dims.size()];
	for(int i = 0; i < dims.size(); i++)
		dataSetDims[i] = dims.at(i);	
	H5::DataSpace dataspace(dims.size(), dataSetDims);
	H5::DataSet dataset = Obj->createDataSet(dataSetName, H5::PredType::NATIVE_FLOAT, dataspace);
}

int pointsCurrTimeStamp(int currTimeStamp, H5Properties *ptr_h5){
		if(currTimeStamp==0) return ptr_h5->numInitInstances; 
		else if(currTimeStamp == ptr_h5->numSnapshots-1) return ptr_h5->numLastBatchInstances;
		else return ptr_h5->numBatchInstances;	
}

void parseLine(pointProperties *ptr_point, csvProperties *ptr_csv){
	std::stringstream sline(*(ptr_point->csvLine));
	std::string tmp;
	unsigned count(0);
	while(std::getline(sline,tmp,',')){ 
		if(count==ptr_csv->gtCol)
			ptr_point->gtValue = std::stof(tmp);
		else
			ptr_point->featVector.push_back(std::stof(tmp));
		count++;
	}
}

void getDataPlacement(pointProperties *ptr_point, H5Properties *ptr_h5){
	if( ptr_point->pointID < ptr_h5->numInitInstances){
		ptr_point->myTimeStamp = 0;
		ptr_point->myPosition = ptr_point->pointID;
	}else{
		ptr_point->myTimeStamp = std::ceil((double)(ptr_point->pointID + 1 - ptr_h5->numInitInstances)
		/ptr_h5->numBatchInstances);
		ptr_point->myPosition = (unsigned)(ptr_point->pointID - ptr_h5->numInitInstances)%ptr_h5->numBatchInstances;
	}
}


void writeData(H5::H5File *file, std::string dataSetName, pointProperties *ptr_point){
	
	// Open TimeStamp (group)
	H5::Group group(file->openGroup("Timestamp_"+std::to_string(ptr_point->myTimeStamp)));

	// Read dataset and datspace
	H5::DataSet dataset = group.openDataSet(dataSetName);
	H5::DataSpace dsSpace = dataset.getSpace();	

	// Extract rank of dataset
	unsigned dims = dsSpace.getSimpleExtentNdims();

	// Dimension size of dataset 
	hsize_t dataDims[dims], offset[dims];
	for(int i = 0; i < dims; i++){dataDims[i] = 0; offset[i] = 0; }
	dsSpace.getSimpleExtentDims( dataDims );	

	// Set memory to write data
	dataDims[0] = 1; offset[0] = ptr_point->myPosition;
	H5::DataSpace memSpace(dims, dataDims);
	dsSpace.selectHyperslab(H5S_SELECT_SET, dataDims, offset);

	// write the data 
	dataset.write( ptr_point->toWrite, H5::PredType::NATIVE_FLOAT, memSpace, dsSpace );
}

void writeFeatures(H5::H5File *h5File, csvProperties *ptr_csv){
	
	// Cast Feature names as standard C arrays and remove the ground truth column
	std::vector<const char*> vectCarrays;
        for (size_t ii = 0; ii < ptr_csv->FeatureNames.size(); ++ii){
		if(ptr_csv->FeatureNames[ii] != ptr_csv->labelColumn)
			vectCarrays.push_back(ptr_csv->FeatureNames[ii].c_str());
	}

	// Set dataspace
	hsize_t dataDims[1] ={vectCarrays.size()};
        H5::DataSpace dsSpace(1, dataDims);

	// Define the variable length datatype
        H5::StrType varLenStrType(H5::PredType::C_S1, H5T_VARIABLE); 

	// Write features as an attribute in the root group
        H5::Attribute attribute = h5File->createAttribute("FeaturesNames", varLenStrType, dsSpace);
        attribute.write(varLenStrType, vectCarrays.data());
}

