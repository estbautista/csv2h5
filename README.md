# csv2h5

C++ program that converts a Comma-Separated Values file (CSV) into an Hierarchical Data Format version 5 (HDF5) file tailored for machine learning applications. 

The HDF5 format is a format to store large amounts of heterogeneous data in an efficient and organized way. For more information please read: [The HDF5 Data Format](https://www.google.com/search?client=safari&rls=en&q=HDF5&ie=UTF-8&oe=UTF-8)
** **
## Usage

The program assumes a cleaned csv file where rows correspond to datapoints and columns, labelled in the top row, correspnd to feature attributes. One of the columns must correspond to the ground truth data upon which a learning algorithm will be trained. The user must call the program using the following command

./csv2h5 < Input CSV File > < Output H5 File> < Name Ground Truth Column > < Optional 1 > < Optional 2 >

* **Input CSV File**: Path to the csv file to convert
* **Output H5 file**: Path to where the hdf5 file will be stored
* **Name of Ground Truth Column**: label of the column that will be tried to be predicted. For examaple file sampleFile.csv this parameter would be: < ClassMembership >
* **Optional 1**: Fraction of datapoints that will be used in the initial timestamp (default = 1)
* **Optional 2**: Number of datapoints that will be used in subsequent timestamps (default = 0)

Optional parameters allow to simulate an evolving setting, where new data arrives. For example, by setting <Optional 1 = 0.5> and <Optional 2 = 10>, then the first half of the datapoints will be placed in a dataset that correspond to timestamp = 0, then the remainder of the dataset will be split in batches of 10 datapoints and each one corresponds to a subsequent timestamp. As a result, the data is organized as timestamps, in which each timestamp is an hdf5 group located in the root group "/". For each timestamp the following datasets (of floating-point datatype) are created:
* **Features**: matrix in which rows are datapoint and columns are feature attributes
* **GroundTruth**: vector that stores the elements of the specified ground truth column
* **IDs**: vector that stores the ID for each datapoint (useful in evolving settings)
* **Prediction**: vector in which predictions by Machine Learning algorithms can be stored
** **
## Instalation 
The code depends on the **HDF5 Library and the C++ API**. You can install it from: https://www.hdfgroup.org/downloads/hdf5/source-code/

Then, to compile the code, change, in the makefile, the library and include paths of the HDF5 library to match your installation. Then, simply run the make command in the command line.
