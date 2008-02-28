analyser_objs = analyser.o 
blaster_objs = 	blaster.o 
splitter_objs = splitter.o 

nfpvr_include_dirs = -I../nfpvrlib
nfpvr_link_dirs    = -L../nfpvrlib

nfpvr_libs = nfpvrlib
nfpvrlib_dir = ../nfpvrlib

all: nfpvrlib analyser blaster splitter 

nfpvrlib:
	make -C $(nfpvrlib_dir)

analyser: $(analyser_objs)
	g++ -o $@ $(analyser_objs) $(nfpvr_link_dirs) -lnfpvr

blaster: $(blaster_objs)
	g++ -o $@ $(blaster_objs) $(nfpvr_link_dirs) -lnfpvr

splitter: $(splitter_objs)
	g++ -o $@ $(splitter_objs) $(nfpvr_link_dirs) -lnfpvr

clean:
	rm -f *.o *.exe

%.o: %.cpp
	g++ -c $< -Wall $(nfpvr_include_dirs)
