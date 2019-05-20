// CSAExtractor.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include "stdafx.h"
#include <filesystem>

struct CSAHeader {
	int          header; // GEEK
	int          unknown[3];
	int          numberOfFiles;
	int          unknown2[2];
};

struct CSAEntry {
	char         filename[128];
	unsigned int offset;
	unsigned int size;
};

std::string splitString(std::string& str, bool file) {

	std::string str_ret;
	int fnd = str.find_last_of("/\\");

	if (file == true) str_ret = str.substr(fnd + 1);
	if (file == false) str_ret = str.substr(0,fnd);
	return str_ret;
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cout << "Usage: CSAExtractor <input>" << std::endl;
		return 1;
	}
	std::ifstream pFile(argv[1], std::ifstream::binary);

	if (!pFile)
	{
		std::cout << "ERROR: Could not open " << argv[1] << "!" << std::endl;
		return 1;
	}

	if (pFile)
	{
		CSAHeader csa;
		pFile.read((char*)&csa, sizeof(CSAHeader));

		if (csa.header != 'KEEG')
		{
			std::cout << "ERROR: " << argv[1] << " is not a valid GEEK archive!" << std::endl;
		}

		
		int currentPos = (int)pFile.tellg();

		for (int i = 0; i < csa.numberOfFiles; i++)
		{
			CSAEntry csaE;
			pFile.read((char*)&csaE, sizeof(CSAEntry));
			pFile.seekg(csaE.offset, pFile.beg);

			// create output
			std::string temp = csaE.filename;
			std::string folder = splitString(temp, false);
			std::string file = splitString(temp, true);
			std::experimental::filesystem::create_directories(folder);
			std::ofstream oFile(temp, std::ofstream::binary);

			// write
			auto dataBuff = std::make_unique<char[]>(csaE.size);
			pFile.read(dataBuff.get(), csaE.size);
			std::cout << "Processing: " << file << std::endl;
			oFile.write(dataBuff.get(), csaE.size);

			currentPos += sizeof(CSAEntry);
			pFile.seekg(currentPos,pFile.beg);
		}
		std::cout << "Finished." << std::endl;
	}
    return 0;
}

