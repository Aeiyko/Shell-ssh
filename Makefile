CFILE_MYPS = $(wildcard src/myps/*.c)
CFILE_COMMANDE = $(wildcard src/commande/*.c)
CFILE_MYLS = $(wildcard src/myls/*.c)
CFILE_MYSSH = $(wildcard src/myssh/*.c)
CFILE_MYSSHD = $(wildcard src/mysshd/*.c)
CFILE_MYSSH_SERVER = $(wildcard src/myssh-server/*.c) src/commande/commands.c src/commande/error.c src/commande/jobs.c src/commande/parser.c
CFILE_TEST_PARSER = src/test/parser_test.c
CFILE_TEST_MEMOIRE = src/test/memoire_test.c src/myssh-server/memoire.c

OFILE_MYPS = $(CFILE_MYPS:src/%.c=build/%.o)
OFILE_COMMANDE = $(CFILE_COMMANDE:src/%.c=build/%.o)
OFILE_MYLS = $(CFILE_MYLS:src/%.c=build/%.o)
OFILE_MYSSH = $(CFILE_MYSSH:src/%.c=build/%.o)
OFILE_MYSSHD = $(CFILE_MYSSHD:src/%.c=build/%.o)
OFILE_MYSSH_SERVER = $(CFILE_MYSSH_SERVER:src/%.c=build/%.o)
OFILE_TEST_PARSER = $(CFILE_TEST_PARSER:src/%.c=build/%.o)
OFILE_TEST_MEMOIRE = $(CFILE_TEST_MEMOIRE:src/%.c=build/%.o)



FOLDERS = $(wildcard src/*)
BUILD_FOLDERS = $(FOLDERS:src/%=build/%)

all: myps mysh myssh mysshd myls myssh_server
ssh: myssh mysshd myssh_server

clean:
	rm -fr bin
	rm -fr build
	@echo "Clean success."

myps: bin/myps
bin/myps: $(OFILE_MYPS)|bin
	gcc $^ -g -Wall -o $@

mysh: bin/mysh
bin/mysh: $(OFILE_COMMANDE)|bin
	gcc $^ -o $@ -g

myls: bin/myls
bin/myls: $(OFILE_MYLS)|bin
	gcc $^ -lm -o $@

myssh: bin/myssh
bin/myssh: $(OFILE_MYSSH)|bin
	gcc $^ -o $@

mysshd: bin/mysshd
bin/mysshd: $(OFILE_MYSSHD)|bin
	gcc $^ -lpthread -lcrypt -o $@

myssh_server: bin/myssh_server
bin/myssh_server: $(OFILE_MYSSH_SERVER)|bin
	gcc $^ -lpthread -o $@

parser: bin/parser
bin/parser: build/commande/parser.o build/test/parser_test.o|bin
	gcc $^ -o $@

memoirepartage: bin/memoirepartage
bin/memoirepartage: $(OFILE_TEST_MEMOIRE)|bin
	gcc $^ -o $@

build/%.o: src/%.c |build
	gcc $^ -c -Wall -o $@ -g

build: $(BUILD_FOLDERS)

build/%:
	@echo "Construction des build $@"
	mkdir -p $@

bin:
	-@mkdir bin
