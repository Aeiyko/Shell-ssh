# Shell-ssh

##POURCENTAGE DE TRAVAIL:
  Melvin Beaussart : 50%
  Alexis Salvetti  : 50%

##COMMENT COMPILER ET UTILISER:
  Nous avons créé un Makefile avec les commandes suivantes:
    - make all
    - make ssh
    - make clean

    - make mysh
    - make myls
    - make myps
    - make myssh
    - make mysshd
    - make myssh_server
    - make memoirepartage
    - make parser

    pour executer :
      faire la commande "bin/NomDeLExec"
      pour mysshd le lancer en sudo.



##FONCTIONNALITES:
  — mysh :
      IMPLEMENTES :
        -Séquencement
        -Wildcards
        -Commandes
        -Redirections
        -Premier et arrière plans

      NON IMPLEMENTES :
        -variables (par manque de temps) mais nous pouvons vous expliquer la manière dont nous aurions procédé :
          Les variables auraient été stockées dans une structure contenant une chaine de caractères pour le nom
          de la variable et une chaine de caractères contenant la valeur associée à cette variable et un entier pour indiquer si la variable est globale ou non.

          Nous aurions donc eu un tableau de ce type de structures (ou une liste chainée pour ne pas avoir une quantité limite de variables crées) en global.

          Avec des fonctions permettant de supprimer une variable (appelé pour un unset), et pour ajouter ou mettre à jour les variables (appelé pour un set).

          Nous aurions remplacé les execvp par des execvpe pour prendre en compte toutes ces nouvelles variables.


      BUGS CONNUS :
        -Sur Arch Linux, les exécutions de commande avec execvp donneront une exec error
        pour des raisons totalement inconnues (nous avons soupçonné la possibilité que le pointeur
        de pile atteigne le pointeur de tas mais nous n'en sommes pas certains)
        d'autant plus que sur Ubuntu (OS, VM, ou WSL et Raspbian), il n'y a pas de problèmes

        -Il n'est pas possible d'effectuer des pipes en utilisant des commandes internes
          exemple :
          -> status | grep 0; renverra un exec error

          par contre il est toujours possible de séquencer
          -> status && ls | more donnera le résultat attendu

        -Il n'est pas possible non plus de faire des redirections dans un enchaînement de pipe
        exemple :
          -> ls >> file.txt | more ou ls | more >> file.txt ne donneront pas les résultats attendus

        par contre, le séquencement ne pose pas de problèmes :
          ->ls | more && ls >> file.txt donnera le résultat attendu

        -Il se peut que, malgré nos efforts pour les empêcher, que des fuites mémoires puissent avoir lieu
        dans certains cas.

  — myssh :
  IMPLEMENTES :
    - Identification
    - Configuration
    - CTRL+C

  BUGS CONNUS :
    mot de passe non sécurisé et ctrl+c qui ne marche pas dans certains cas.


  — mysshd :
  IMPLEMENTES :
      -Lancer un serveur myssh-server
      -Multi-threads

  BUGS CONNUS :
      Nous avons quelque bugs de ctrl+c pour certains cas donc nous les avons commenté.
      Si vous avez une segfault il faut decommenter ou commenter la premiere ligne GNU SOURCE car cela depend de l'OS sur lequel vous executé le programme


  — myssh-server :
  IMPLEMENTES :
    - Généralité et format du protocole
    - Authentification
    - Envoie d'une commande
    - Envoi d'un signal


  NON IMPLEMENTES :



  BUGS CONNUS :
    La première commande ne fonctionne jamais et renvoie un code erreur venant du ssh(donc ça ne vient pas du serveur lui meme mais du exec_cmd).


  — myls : tout est implémenté sauf certains cas que nous n'avons pas vu lors de nos tests de ls -l (un membre
    d'un autre groupe nous a fait savoir que pour un certain type de fichier il fallait afficher un + après les
    permissions mais nous n'avons jamais observé un tel cas, pourtant nous avons fouillé récursivement la racine
    de notre WSL et notre VM ubuntu);

    BUGS CONNUS :
    Nous avons dû commenter certains free car lors d'un test de bin/myls -Ra à partir du dossier Shell-ssh,
    il y a eut de sérieux bugs d'affichage (que nous n'avons pas eut en executant cette commande avec valgrind).
    En commentant ces free, le problème d'affichage n'est plus présent mais il y a des fuites mémoires que nous
    n'avons pas réussi à résoudre  

  — myps :
    Tout a était implémenté sauf le tty, le time et l'affichage qui n'est pas adpaté.

QUELQUES REMARQUES:

  -Pour mysh quand aucun processus n'est en premier plan, si l'on effectue un CTRL+C, toutes les tâches de fond
  sont tuées avant de demander si l'utilisateur souhaite quitter (nous avons interprété le sujet de cette façon
  mais nous avons quelques doutes concernant notre compréhension de cette partie, dans le doute nous
  vous prions de bien vouloir nous en excuser).

  - Nous n'avons pas eu le temps d'ajouter la fonctionnalité de mémoire partagé au shell mais elle a était développé et vous pouvez la tester.

  -Nous avons clairement souffert d'un manque de temps pour finir ce projet, nous n'avons pas relâché nos
  efforts, nous avons passé toutes nos vacances dessus mais nous sommes conscient que nous aurions pû vous rendre
  un projet bien plus aboutie si le temps et le contexte dans lequel nous avons effectué ce projet nous le
  permettait. Nous avons fait de notre mieux et nous vous prions de bien vouloir nous excuser si notre version ne
  correspond pas à vos attentes.
