memory: memory.c
    gcc memory.c -Wall -g -o memory

install:
    @echo "L’ installation necessite les droits root"
    chown root ./memory
	#e.g. on vas executer avec le EUID de root.
    chmod u+s ./memory
    @echo " Installation reussie"