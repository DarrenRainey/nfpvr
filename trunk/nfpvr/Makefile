nfpvr_objs = \
	ProgramOptions.o \
	nfpvr.o

nfpvr_libs = nfpvrlib

nfpvr_include_dirs = -I../nfpvrlib
nfpvr_link_dirs    = -L../nfpvrlib

nfpvrlib_dir = ../nfpvrlib


all: nfpvr

nfpvrlib:
	make -C $(nfpvrlib_dir)

nfpvr: $(nfpvr_libs) $(nfpvr_objs)
	g++ -o $@ $(nfpvr_objs) $(nfpvr_link_dirs) -lnfpvr

clean:
	rm -f *.o *.exe

%.o: %.cpp
	g++ -c $< -Wall $(nfpvr_include_dirs)
