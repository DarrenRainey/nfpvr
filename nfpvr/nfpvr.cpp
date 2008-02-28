#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdexcept>

#include "NfpvrLib.h"
#include "ProgramOptions.h"

const char* VersionString = "0.1.4";

void printVersion(const char* program, const char* version)
{
	printf(	"%s version %s.\n"
			"\n"
			"Copyright 2008 GNU Public License (GPL)\n"
			"This is free software. There is NO warranty.\n"
			"I am NOT responsible if your computer breaks down, your cat dies or if\n"
			"your family members become zombies. Enjoy.\n",
			program, version);
}

void printHeader(const char* program, const char* version)
{
	printf("%s %s, run \"%s --help\" for help\n", program, version, program);
}

void printUsage(const char* program)
{
	printf("Usage: %s [options]\n"
		   "\n"
		   "Valid options are:\n"
		   "  -a, --audio                 Handle audio stream\n"
		   "  -b, --buffer                Buffer output mpeg write calls\n"
		   "  -h, --help                  Print this help message\n"
		   "  -n, --nompeg                Don't compose the mpeg files\n"
		   "  -o, --outputdir <directory> Output directory where to save mpeg files\n"
		   "  -p, --port <port #>         UDP port on which to listen to (default: %d)\n"
		   "  -r, --readraw <filename>    Reads raw data from a file instead of the network\n"
		   "  -v, --verbose               Run in verbose mode\n"
		   "  -w, --writeraw <filename>   Writes raw data from the network to a file\n"
		   "  -x, --version               Print the version number\n",
		   program,
		   ProgramOptions::DEFAULT_UDP_PORT);
}

bool parseOptions(ProgramOptions& options, int argc, char* argv[])
{
	for (int x=1; x<argc; x++)
	{
		if (!strcmp(argv[x], "-w") || 
			!strcmp(argv[x], "--writeraw"))
		{
			x++;

			if (x<argc)
			{
				options._writeRawFilename = argv[x];
				options._writeRaw = true;
			}
			else
				return false;
		}
		else if (!strcmp(argv[x], "-r") || 
				 !strcmp(argv[x], "--readraw"))
		{
			x++;

			if (x<argc)
			{
				options._readRaw = true;
				options._readRawFilename = argv[x];
			}
			else
				return false;
		}
		else if (!strcmp(argv[x], "-p") || 
				 !strcmp(argv[x], "--port"))
		{
			x++;

			if (x<argc)
			{
				options._udpPort = atoi(argv[x]);
			}
			else
				return false;
		}
		else if (!strcmp(argv[x], "-o") || 
				 !strcmp(argv[x], "--outputdir"))
		{
			x++;

			if (x<argc)
			{
				options._outputDirectory = argv[x];
			}
			else
				return false;
		}
		else if (!strcmp(argv[x], "-a") || 
				 !strcmp(argv[x], "--audio"))
		{
			options._handleAudio = false;
		}
		else if (!strcmp(argv[x], "-b") || 
				 !strcmp(argv[x], "--buffer"))
		{
			options._bufferOutput = false;
		}
		else if (!strcmp(argv[x], "-n") || 
				 !strcmp(argv[x], "--nompeg"))
		{
			options._writeMpeg = false;
		}
		else if (!strcmp(argv[x], "-v") || 
				 !strcmp(argv[x], "--verbose"))
		{
			options._verbose = true;
		}
		else if (!strcmp(argv[x], "-x") || 
				 !strcmp(argv[x], "--version"))
		{
			options._version = true;
		}
		else if (!strcmp(argv[x], "-h") || 
				 !strcmp(argv[x], "--help"))
		{
			return false;
		}
		else
		{
			return false;
		}
	}
	return true;
}

class NfpvrInterfaceNetwork: public INfpvrInterfaceNetwork
{
public:
	Options     _options;
	const char* _outputDirectory;

	NfpvrInterfaceNetwork(ProgramOptions& options)
	{
		_options._writeRaw = options._writeRaw;
		_options._writeMpeg = options._writeMpeg;
		_options._handleAudio = options._handleAudio;
		_options._bufferOutput = options._bufferOutput;
		_options._writeRawFilename = options._writeRawFilename;
		_options._outputDirectory = options._outputDirectory;
	}

	void notify(NotifyLevel level, const char* format, ...)
	{
		static char message[1024];
		va_list list;
		va_start(list, format);
		vsnprintf(message, sizeof(message), format, list);
		va_end(list);

		switch (level)
		{
			case NotifyWarning: printf("warning: "); break;
			case NotifyError: throw message;
			default: break;
		}

		printf(message);
		printf("\n");
	}

	const Options& getOptions()
	{
		return _options;		
	}

	void networkListenComplete(nfpvr_socket_t theSocket)
	{
		// do nothing
	}

	int networkReceive(nfpvr_socket_t theSocket, nfpvr_sockaddr_t* theSender, uint8* data, const int length)
	{
		int received = 0;
		const int senderSize = sizeof(struct sockaddr_in);

		do
		{
			received = recvfrom(theSocket, (char*)(data), length, 0, 
				(nfpvr_sockaddr_t*)theSender, (nfpvr_socklen_t*)&senderSize);
		} while (received == NFPVR_SOCKET_ERROR);

		return received;
	}
};

int main(int argc, char* argv[])
{
	ProgramOptions options;
	if (parseOptions(options, argc, argv))
	{
		NfpvrInterfaceNetwork interface(options);
		if (options._version)
		{
			printVersion(argv[0], VersionString);
		}
		else
		{
			printHeader(argv[0], VersionString);

			try
			{
				if (options._readRaw)
				{
					NfpvrLibReadFromFile(interface, options._readRawFilename);
				}
				else
				{
					NfpvrLibReadFromUdp(interface, options._udpPort);
				}
			}
			catch(const char* message)
			{
				printf("error: %s\n", message);
			}
		}
	}
	else
	{
		printUsage(argv[0]);
	}

	return 0;
}
