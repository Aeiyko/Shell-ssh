CFILE_MYPS = $(wildcard src/myps/*.c)
CFILE_COMMANDE = $(wildcard src/commande/*.c)
CFILE_MYLS = $(wildcard src/myls/*.c)
CFILE_MYSSH = $(wildcard src/myssh/*.c)
CFILE_MYSSHD = $(wildcard src/mysshd/*.c)

OFILE_MYPS = $(CFILE_MYPS:src/%.c=build/%.o)
OFILE_COMMANDE = $(CFILE_COMMANDE:src/%.c=build/%.o)
OFILE_MYLS = $(CFILE_MYLS:src/%.c=build/%.o)
OFILE_MYSSH = $(CFILE_MYSSH:src/%.c=build/%.o)
OFILE_MYSSHD = $(CFILE_MYSSHD:src/%.c=build/%.o)

FOLDERS = $(wildcard src/*)
BUILD_FOLDERS = $(FOLDERS:src/%=build/%)

all: myps commande myssh mysshd myls

clean:
	rm -fr bin
	rm -fr build
	@echo "Clean success."

myps: bin/myps
bin/myps: $(OFILE_MYPS)|bin
	gcc $^ -o $@

commande: bin/commande
bin/commande: $(OFILE_COMMANDE)|bin
	gcc $^ -o $@

myls: bin/myls
bin/myls: $(OFILE_MYLS)|bin
	gcc $^ -lm -o $@

myssh: bin/myssh
bin/myssh: $(OFILE_MYSSH)|bin
	gcc $^ -o $@

mysshd: bin/mysshd
bin/mysshd: $(OFILE_MYSSHD)|bin
	gcc $^ -lcrypt -o $@

build/%.o: src/%.c |build
	gcc $^ -c -Wall -o $@

build: $(BUILD_FOLDERS)

build/%:
	mkdir -p $@

bin:
	-@mkdir bin