// csatool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <memory>
#include <fstream>
#include <filesystem>

#include "filef.h"
#include "csa.h"


enum eModes {
	MODE_EXTRACT  = 1,
	MODE_CREATE,
	PARAM_STAR
};

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		std::cout << "CSATool - extract and create .csa archives\n"
			"Usage: csatool <params> <input>\n"
			"Params:\n"
			"   -e         Extracts input archive\n"
			"   -c         Creates archive from input folder\n"
			""
			"   -o <name>  Specifies output filename/folder\n"
			"Example:\n"
			"csatool -e pack.csa\n"
			"csatool -e -o output_folder pack.csa\n"
			"csatool -c -o new.csa filesfolder\n";
		return 0;
	}


	int mode = 0;
	int param = 0;
	std::string o_param;

	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'e': mode = MODE_EXTRACT;
			break;
		case 'c': mode = MODE_CREATE;
			break;
		case 's': param = PARAM_STAR;
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}

	if (mode == MODE_EXTRACT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}
		
		if (pFile)
		{
			csa_header csa;
			pFile.read((char*)&csa, sizeof(csa_header));

			if (!(csa.header == 'KEEG' || csa.version == 256))
			{
				std::cout << "ERROR: " << argv[argc - 1] << " is not a valid CSA archive!" << std::endl;
				return 1;
			}

			std::unique_ptr<csa_entry[]> entries = std::make_unique<csa_entry[]>(csa.files);

			for (int i = 0; i < csa.files; i++)
			{
				pFile.read((char*)&entries[i], sizeof(csa_entry));
			}

			if (!o_param.empty())
			{
				if (!std::experimental::filesystem::exists(o_param))
					std::experimental::filesystem::create_directory(o_param);

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
			}

			for (int i = 0; i < csa.files; i++)
			{
				int dataSize = entries[i].size;
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

				pFile.seekg(entries[i].offset, pFile.beg);
				pFile.read(dataBuff.get(), dataSize);

				std::string output = entries[i].fileName;

				std::cout << "Processing: " << output << std::endl;

				if (checkSlash(output))
					std::experimental::filesystem::create_directories(splitString(output, false));

				std::ofstream oFile(output, std::ofstream::binary);
				oFile.write(dataBuff.get(), dataSize);

			}
			std::cout << "Finished." << std::endl;
		}
	}
	if (mode == MODE_CREATE)
	{
		if (!std::experimental::filesystem::exists(argv[argc - 1]))
		{
			std::cout << "ERROR: Could not open directory: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (std::experimental::filesystem::exists(argv[argc - 1]))
		{
			int filesFound = 0;

			for (const auto & file : std::experimental::filesystem::recursive_directory_iterator(argv[argc - 1]))
			{
				if (file.path().has_extension())
					filesFound++;
			}

			std::unique_ptr<std::string[]> filePaths = std::make_unique<std::string[]>(filesFound);
			std::unique_ptr<int[]> sizes = std::make_unique<int[]>(filesFound);

			int i = 0;
			for (const auto & file : std::experimental::filesystem::recursive_directory_iterator(argv[argc - 1]))
			{
				if (file.path().has_extension())
				{
					filePaths[i] = file.path().string();
					std::ifstream tFile(filePaths[i], std::ifstream::binary);
					if (tFile)
					{
						sizes[i] = (int)getSizeToEnd(tFile);
					}
					i++;
				}
			}

			// create archive
			csa_header csa;
			csa.files = filesFound;
			csa.version = 256;
			csa.header = 'KEEG';
			csa.mainStructSize = sizeof(csa_header) + (sizeof(csa_entry) * filesFound) - sizeof(int) * 2;
			csa.firstFileOffset = sizeof(csa_header) + (sizeof(csa_entry) * filesFound) - sizeof(int) * 2;
			csa.unknown = 0;
			if (param == PARAM_STAR)
				csa.gameLabel = STAR_GAMES_LABEL;
			else
				csa.gameLabel = NOSFERATU_FBI_LABEL;

			std::string output;

			if (o_param.empty())
				output = "new.csa";
			else
				output = o_param;

			std::ofstream oArchive(o_param, std::ofstream::binary);
			oArchive.write((char*)&csa, sizeof(csa_header));
			int baseOffset = sizeof(csa_header) + (sizeof(csa_entry) * filesFound);
			for (int i = 0; i < csa.files; i++)
			{
				csa_entry entry;
				sprintf(entry.fileName, "%s", filePaths[i].c_str());
				entry.offset = baseOffset;
				entry.size = sizes[i];
				oArchive.write((char*)&entry, sizeof(csa_entry));
				baseOffset += sizes[i];
			}

			for (int i = 0; i < csa.files; i++)
			{
				std::ifstream pFile(filePaths[i], std::ifstream::binary);

				if (!pFile)
				{
					std::cout << "ERROR: Could not open " << filePaths[i] << std::endl;
					return 1;
				}

				if (pFile)
				{
					int dataSize = sizes[i];
					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
					pFile.read(dataBuff.get(), dataSize);
					std::cout << "Processing: " << filePaths[i] << std::endl;;
					oArchive.write(dataBuff.get(), dataSize);
				}
			}
			std::cout << "Finished. Saved to: " << output << std::endl;
		}

		
	}
}
