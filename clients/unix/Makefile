COSMO := cosmopolitan/o

all: client.com

clean:
	rm -f client.com.dbg client.com

${COSMO}/cosmopolitan.h:
	wget https://justine.lol/cosmopolitan/cosmopolitan-1.0.tar.gz
	tar xf cosmopolitan-1.0.tar.gz
	rm cosmopolitan-1.0.tar.gz
	cd cosmopolitan
	make -j16 all o/cosmopolitan.h

client.com.dbg: client.c base64.h ${COSMO}/cosmopolitan.h
	gcc -g -Os -static -nostdlib -nostdinc -fno-pie -no-pie -mno-red-zone \
	  -fno-omit-frame-pointer -pg -mnop-mcount \
	  -o client.com.dbg client.c -fuse-ld=bfd -Wl,-T,${COSMO}/ape/ape.lds \
	  -I ${COSMO} -include ${COSMO}/cosmopolitan.h ${COSMO}/libc/crt/crt.o ${COSMO}/ape/ape.o ${COSMO}/cosmopolitan.a

client.com: client.com.dbg
	objcopy -S -O binary client.com.dbg client.com
