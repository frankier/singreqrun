.INTERMEDIATE: cosmopolitan-1.0.tar.gz

COSMO := cosmopolitan/o

CFLAGS := -Os -static -nostdlib -nostdinc -fno-pie -no-pie -mno-red-zone \
-fno-omit-frame-pointer -pg -mnop-mcount

LFLAGS := -fuse-ld=bfd -Wl,-T,${COSMO}/ape/ape.lds \
-I ${COSMO} -include ${COSMO}/cosmopolitan.h ${COSMO}/libc/crt/crt.o \
${COSMO}/ape/ape.o ${COSMO}/cosmopolitan.a

all: clients/unix/client.com

debug: clients/unix/client.com.dbg

clean:
	rm -f clients/unix/client.com.dbg clients/unix/client.com

cosmopolitan-1.0.tar.gz:
	wget https://justine.lol/cosmopolitan/cosmopolitan-1.0.tar.gz

cosmopolitan/Makefile: cosmopolitan-1.0.tar.gz
	tar xf cosmopolitan-1.0.tar.gz \
	  && touch --reference=cosmopolitan-1.0.tar.gz cosmopolitan/Makefile

${COSMO}/cosmopolitan.h: cosmopolitan/Makefile
	cd cosmopolitan && make -j16 all o/cosmopolitan.h

clients/unix/client.com.dbg: clients/unix/client.c clients/unix/base64.h ${COSMO}/cosmopolitan.h
	gcc -g ${CFLAGS} -o clients/unix/client.com.dbg clients/unix/client.c ${LFLAGS}

clients/unix/client.com:  clients/unix/client.c clients/unix/base64.h ${COSMO}/cosmopolitan.h
	gcc ${CFLAGS} -o clients/unix/client.com clients/unix/client.c ${LFLAGS}
