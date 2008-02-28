libnfpvr_objs = \
	Bitfield.o \
	FilePacketReader.o \
	MpegHeader.o \
	PacketHandler.o \
	PesStreamHandler.o \
	Serializable.o \
	UdpPacketReader.o \
	NfpvrLib.o \
	Utility.o	

all: libnfpvr.a

libnfpvr.a: $(libnfpvr_objs)
	ar rcs libnfpvr.a $(libnfpvr_objs)

clean:
	rm -f *.a *.o

%.o: %.cpp
	g++ -c $< -Wall
