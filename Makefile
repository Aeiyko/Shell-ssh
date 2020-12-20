all: file 

clean:
	@rm -fr bin
	@echo "Clean success."

ps:
	gcc src/ps.c src/error.c

file:
	mkdir bin