COSMO := cosmopolitan/o

CFLAGS := "-g -Os -static -nostdlib -nostdinc -fno-pie -no-pie -mno-red-zone \
	   -fno-omit-frame-pointer -pg -mnop-mcount \
	   -fuse-ld=bfd -Wl,-T,${COSMO}/ape/ape.lds \
	   -I ${COSMO} -include ${COSMO}/cosmopolitan.h ${COSMO}/libc/crt/crt.o \
	   ${COSMO}/ape/ape.o ${COSMO}/cosmopolitan.a"

all: clients/unix/client.com

clean:
	rm -f clients/unix/client.com.dbg clients/unix/client.com

${COSMO}/cosmopolitan.h:
	wget https://justine.lol/cosmopolitan/cosmopolitan-1.0.tar.gz
	tar xf cosmopolitan-1.0.tar.gz
	rm cosmopolitan-1.0.tar.gz
	cd cosmopolitan
	make -j16 all o/cosmopolitan.h

clients/unix/client.com.dbg: clients/unix/client.c clients/unix/base64.h ${COSMO}/cosmopolitan.h
	gcc ${CFLAGS} -o clients/unix/client.com.dbg clients/unix/client.c

clients/unix/client.com: clients/unix/client.com.dbg
	objcopy -S -O binary clients/unix/client.com.dbg clients/unix/client.com

servers/static_unix/server.com.dbg: servers/static_unix/server.c ${COSMO}/cosmopolitan.h
	gcc ${CFLAGS} -o servers/static_unix/client.com.dbg servers/static_unix/server.c

servers/static_unix/server.com: servers/static_unix/server.com.dbg
	objcopy -S -O binary servers/static_unix/server.com.dbg servers/static_unix/server.com
