## What is BR Mixer?

BR Mixer reads .mix files used by the 1997 Westwood game Blade Runner. The mix format acts as a container for various other file formats:

* VQA - Video Files
* SHP - Sprites
* SET - Set / Location data
* DAT - Container holding additional files
* TRE - String resources
* AUD - Audio Files
* TLK - Speech Files
* FON - Fonts
* IMG - Image Files (?)

While it is possible to list all files contained in a mix file, BR Mixer does not yet fully support all of these formats and will only be able to process some of them.

### Requirements

BR Mixer requires the following libraries at compile-time:

* libpng (1.6.16)

### Building BR Mixer

To build BR mixer on Windows, open the solution under \vc2015 in Visual Studio 2015. It should be possible to build the solution in earlier versions as well.

In order to build the program on Linux, run:

	make
	
This will generate an application binary in the \bin directory.

### Running BR Mixer

In order to run BR Mixer on Windows, navigate to the \bin directory and run the following from the command line:

	BRMixer.exe <path to mix file>
	
Similarly, on Linux, run:

	brmixer <path to mix file>
	
### Examples

To load the contents of STARTUP.MIX, run:

	C:\br-mixer\bin>BRMixer.exe "C:\Games\Blade Runner\STARTUP.MIX"

This will load up the mix file:
	
	Loading known filenames from: ../data/filenames.txt
	Loading MIX file: C:\Games\Blade Runner\STARTUP.MIX
	Found 137 entries.
	[S]how all files, [L]oad file entry, [E]xtract file entry, or [Q]uit:
	
Once the file has been loaded, enter 'S' to list all entries contained in the mix file:

	[S]how all files, [L]oad file entry, [E]xtract file entry, or [Q]uit: S
	ID              Name            Offset          Size            Type
	 802e5dd        GAMEINFO.DAT        1650            6626           5
	 9413dc5        BRLOGO_E.VQA     1610018          256316           1
	...

To load an individual file, enter "L" followed by the name of the entry. For example, to show the contents of GAMEINFO.DAT, enter:

	[S]how all files, [L]oad file entry, [E]xtract file entry, or [Q]uit: L
	Enter the file to load: GAMEINFO.DAT
	
To extract a file to disk, enter "E" followed by the name of the entry: 

	[S]how all files, [L]oad file entry, [E]xtract file entry, or [Q]uit: E
	Enter the file to extract: GAMEINFO.DAT
	Extracting file to: ../data/GAMEINFO.DAT
