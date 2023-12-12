#include "DataFile.h"
#include <fstream>
#include <iostream>
using namespace std;

DataFile::DataFile()
{
	recordCount = 0;
	filename.clear();
}

DataFile::~DataFile()
{
	Clear();
}

void DataFile::AddRecord(string imageFilename, string name, int age)
{
	// load images
	Image i = LoadImage(imageFilename.c_str());

	Record* r = new Record;
	r->image = i;
	r->name = name;
	r->age = age;

	records.push_back(r);
	recordCount++;
}

DataFile::Record* DataFile::GetRecord(int index)
{
	// check the file has been initalized
	if (filename.empty() && index >= records.size()) { throw "A file has not been initalized; call `void Load(string)` to initialize a file"; }
	// check the index is not out of bounds
	if (index > recordCount) { throw "`index` out of bounds"; }
	// load more records if they are not enough
	while (index >= records.size()) {
		LoadRecord(records.size());
	}
	return records[index];
}

void DataFile::Save(string filename)
{
	ofstream outfile(filename, ios::binary);

	// save record count
	int recordCount = records.size();
	outfile.write((char*)&recordCount, sizeof(int));

	// save records
	for (int i = 0; i < recordCount; i++)
	{
		// convert image to char[]
		Color* imgdata = GetImageData(records[i]->image);
		// reference data for reading later
		int imageSize = sizeof(Color) * records[i]->image.width * records[i]->image.height;
		int nameSize = records[i]->name.length();
		int ageSize = sizeof(int);

		// write data
		outfile.write((char*)&records[i]->image.width, sizeof(int));
		outfile.write((char*)&records[i]->image.height, sizeof(int));
		outfile.write((char*)&nameSize, sizeof(int));
		outfile.write((char*)&ageSize, sizeof(int));
		outfile.write((char*)imgdata, imageSize);
		outfile.write((char*)records[i]->name.c_str(), nameSize);
		outfile.write((char*)&records[i]->age, ageSize);
	}

	outfile.close();
}

void DataFile::Load(string filename)
{
	ifstream infile(filename, ios::binary);

	// save how many records exist
	recordCount = 0;
	infile.read((char*)&recordCount, sizeof(int));
	// save filename
	this->filename = filename;

	infile.close();
}

void DataFile::LoadRecord(int index)
{
	if (filename == "") { throw "A file has not been initalized; call `void Load(string)` to initialize a file"; }
	// open file stream
	ifstream infile(filename, ios::binary);

	// check record count
	recordCount = 0;
	infile.read((char*)&recordCount, sizeof(int));
	if (index > recordCount && index < 0) {
		infile.close();
		throw "`index` out of bounds";
	}

	// data sizes
	int width = 0, height = 0, imageSize = 0;
	int nameSize = 0;
	int ageSize = 0;

	// skip records
	for (int i = 0; i < index; i++) {
		// read data sizes
		infile.read((char*)&width, sizeof(int));
		infile.read((char*)&height, sizeof(int));
		imageSize = sizeof(Color) * width * height;
		infile.read((char*)&nameSize, sizeof(int));
		infile.read((char*)&ageSize, sizeof(int));
		// skip data
		streampos current = infile.tellg();
		streamsize dataSize = imageSize + (nameSize * sizeof(char)) + ageSize;
		streampos next = current + dataSize;
		infile.seekg(next);
	}

	// read record
	infile.read((char*)&width, sizeof(int));
	infile.read((char*)&height, sizeof(int));
	imageSize = sizeof(Color) * width * height;
	infile.read((char*)&nameSize, sizeof(int));
	infile.read((char*)&ageSize, sizeof(int));
	// raw data
	char* imgdata = new char[imageSize];
	char* name = new char[nameSize];
	int age = 0;
	// read data
	infile.read(imgdata, imageSize);
	infile.read((char*)name, nameSize);
	infile.read((char*)&age, ageSize);
	// record
	Record* record = new Record();
	record->image = LoadImageEx((Color*)imgdata, width, height);
	record->name = string(name);
	record->name.resize(nameSize); // resize string to correct size
	record->age = age;
	records.push_back(record);

	// clean-up
	delete[] imgdata;
	delete[] name;
	infile.close();

	return;
}

void DataFile::Clear()
{
	for (int i = 0; i < records.size(); i++)
	{
		delete records[i];
	}
	records.clear();
	recordCount = 0;
}