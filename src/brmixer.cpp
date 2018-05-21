#include <iostream>
#include "mixfile.h"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: BRMixer.exe <MIX file>" << std::endl;
		return 1;
	}

	try
	{
		if (!MixFile::load_filenames("../data/filenames.txt"))
		{
			std::cerr << "Unable to load known filenames - file name resolution disabled." << std::endl;
		}

		MixFile file(argv[1]);
		std::string input;
		while (true)
		{
			char c;
			std::cout << "[S]how all files, [L]oad file entry, [E]xtract file entry, or [Q]uit: ";
			std::cin >> c;
			if (c == 's' || c == 'S')
			{
				file.list_files();
			}
			else if (c == 'l' || c == 'L')
			{
				std::cout << "Enter the file to load: ";
				std::cin >> input;
				file.load_file(input);
			}
			else if (c == 'e' || c == 'E')
			{
				std::cout << "Enter the file to extract: ";
				std::cin >> input;
				file.extract_file(input);
			}
			else if (c == 'q' || c == 'Q')
			{
				break;
			}
		}
		return 0;
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}
