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
	
