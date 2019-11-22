YFLAGS = -vd
BIN = /usr/local/bin/
MAN = /usr/local/man/man1/
SPSRCS = ss_cazm.c ss_hspice.c ss_spice2.c ss_spice3.c ss_spicestream.c ss_stubs.c ss_wavefile.c
SPOBJS = ss_cazm.o ss_hspice.o ss_spice2.o ss_spice3.o ss_spicestream.o ss_stubs.o ss_wavefile.o 
SRCS = sp.c post.y rlgetc.c datum.c symbol.c init.c script.c com_ls.c com_ci.c graph.c license.c $(SPSRCS)
OBJS = post.o rlgetc.o datum.o symbol.o init.o script.o com_ls.o com_ci.o graph.o license.o $(SPOBJS)
EXAMPLES = aoi.W.tr0 nand.N.tr0 pd1.N.tr0 quickINV.tr0 quickTRAN.tr0
HEADERS =  rlgetc.h datum.h script.h post.h y.tab.h  symbol.h \
ss_intern.h ss_spice2.h ss_spicestream.h ss_wavefile.h
DEMOS=scriptdemo
TARS = HOWTO README COPYING makefile $(SRCS) $(DEMOS) $(HEADERS) $(EXTRAS) $(EXAMPLES)
CCFLAGS = -ggdb -Wall

all:	post sp

post:	$(OBJS) 
	cc $(CCFLAGS) $(OBJS) -lm -o post -lreadline -g -lncurses

sp: 	$(SPOBJS) sp.o
	cc sp.o $(CCFLAGS) $(SPOBJS) -lm -o sp -lreadline -g -lncurses

.c.o:
	cc $(CCFLAGS) -c $*.c   

x.tab.h: y.tab.h
	cmp -s x.tab.h y.tab.h || cp y.tab.h x.tab.h

clean:
	rm -f $(OBJS) 
	rm -f y.tab.c x.tab.c y.output
	rm -f post
	rm -f sp

install: post sp
	cp  post $(BIN)/post
	cp  sp $(BIN)/sp
	cp  post.1 $(MAN)/post.1
	mkdir -p /usr/local/src/cmd/post
	cp * /usr/local/src/cmd/post

tar: $(TARS)
	(  d=`date +%F`;\
	   q=`pwd`;\
	   p=`basename $$q`;\
	   rm -rf $$p$$d;\
	   mkdir $$p$$d;\
	   cp -rp $(TARS) $$p$$d;\
	   tar czvf - $$p$$d >$$p$$d.tar.gz;\
	)


depend: ${OBJ}
	cp makefile makefile.bak
	sed -n -e '1,/^# DO NOT DELETE OR MODIFY THIS LINE/p' makefile \
                > newmakefile
	grep '^#include[ 	]*"' *.c \
                | sed -e 's/:#include[  "]*\([a-z0-9\._A-Z]*\).*/: \1/' \
                | sed -e 's/\.c:/.o:/' \
                | sort | uniq >> newmakefile
	mv makefile makefile.bak
	mv newmakefile makefile

#-----------------------------------------------------------------
# DO NOT PUT ANY DEPENDENCIES AFTER THE NEXT LINE -- they will go away
# DO NOT DELETE OR MODIFY THIS LINE -- make depend uses it
com_ci_new.o: post.h
com_ci_new.o: ss_wavefile.h
com_ci_new.o: y.tab.h
com_ci_old.o: post.h
com_ci_old.o: ss_wavefile.h
com_ci_old.o: y.tab.h
com_ci.o: post.h
com_ci.o: ss_wavefile.h
com_ci.o: y.tab.h
com_ls.o: post.h
datum.o: post.h
gnew.o: post.h
gnew.o: script.h
gold.o: post.h
gold.o: script.h
g.o: post.h
g.o: script.h
graph.old.o: post.h
graph.old.o: script.h
graph.o: post.h
graph.o: script.h
init.o: post.h
init.o: y.tab.h
rlgetc.o: rlgetc.h
script.o: script.h
sp.o: ss_wavefile.h
ss_cazm.o: ss_intern.h
ss_cazm.o: ss_spicestream.h
ss_hspice.o: ss_intern.h
ss_hspice.o: ss_spicestream.h
ss_spice2.o: ss_intern.h
ss_spice2.o: ss_spice2.h
ss_spice2.o: ss_spicestream.h
ss_spice3.o: ss_intern.h
ss_spice3.o: ss_spicestream.h
ss_spicestream.o: ss_intern.h
ss_spicestream.o: ss_spicestream.h
ss_wavefile.o: ss_intern.h
ss_wavefile.o: ss_wavefile.h
symbol.o: post.h
symbol.o: y.tab.h
