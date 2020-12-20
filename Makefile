all: ps commande

clean:
	@rm -fr bin
	@echo "Clean success."

ps: file
	@gcc src/ps/ps.c src/ps/error.c -o bin/myps

commande: file
	@gcc src/commande/commande.c src/commande/error.c -o bin/commande

myls: file
	@gcc -lm src/myls/myls.c -o bin/myls

file:
	-@mkdir -p bin