all:
	make -C nfpvr
	make -C utils

clean:
	make clean -C nfpvr
	make clean -C utils

