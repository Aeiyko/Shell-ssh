all: file ps commande

clean:
	@rm -fr bin
	@echo "Clean success."

ps:
	gcc src/ps/ps.c src/ps/error.c -o bin/myps

commande:
	gcc src/commande/commande.c src/commande/error.c -o bin/commande

myls:
	gcc -lm src/myls/myls.c -o bin/myls

file:
	mkdir bin