#include <iostream>
#include "mixfile.h"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: BRMixer.exe <MIX file>" << std::endl;
		return 1;
	}

	MixFile::loadKnownIds("../data/id.txt");

	MixFile file(argv[1]);
	std::string input;
	while (true)
	{
		std::cout << "[L]ist files, enter file name to load, or [Q]uit: ";
		std::cin >> input;

		if (input == "q")
			break;
		else if (input == "l")
			file.listFiles();
		else
			file.loadByName(input);
	}
	return 0;
}