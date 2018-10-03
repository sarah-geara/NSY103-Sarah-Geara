gccflags=-g -w
linklibs=-lpthread
libshared=out/list.o out/tcpconnection.o out/tcplistener.o

default: all

_out:
	mkdir -p out

_shared: _out
	gcc ${gccflags} -c shared/list.c -o out/list.o ${linklibs}
	gcc ${gccflags} -c shared/tcpconnection.c -o out/tcpconnection.o ${linklibs}
	gcc ${gccflags} -c shared/tcplistener.c -o out/tcplistener.o ${linklibs}

_agora: _shared
	gcc ${gccflags} -c agora/agora.c -o out/agora.o ${linklibs}
	gcc ${gccflags} -o out/agora out/agora.o ${libshared} ${linklibs}

_cameneon: _shared
	gcc ${gccflags} -c cameneon/cameneon.c -o out/cameneon.o ${linklibs}
	gcc ${gccflags} -o out/cameneon out/cameneon.o ${libshared} ${linklibs}

_mail: _shared
	gcc ${gccflags} -c mail/mail.c -o out/mail.o ${linklibs}
	gcc ${gccflags} -o out/mail out/mail.o ${libshared} ${linklibs}

cleanintermediates:
	rm -r out/*.o

clean:
	rm -rf out

all: _agora _cameneon _mail cleanintermediates
