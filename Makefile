all:
	make -C nfpvrlib
	make -C nfpvr
	make -C utils

clean:
	make clean -C nfpvrlib
	make clean -C nfpvr
	make clean -C utils

