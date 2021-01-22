all: somename.c
	gcc somename.c -Wall -o somename

#CHMOD WHO WHAT WHICH
#user group other all, remove add or set to, read write execute.
#chmod ugoa -+= rwx

#on a besoin des droits root sur le fichier, car on effectue un chroot,
#et cela requiert être root.
install:
	@echo "L'installation necessite les droits root"
	#chown modifie le propriétaire du fichier somename
	#e.g. root devient le propriétaire de ./somename
	chown root ./somename 

	#chmod modifie les bits du mode du fichier
	#u+s vas set le bit S_UID du fichier, ce qui veut dire que 
	#dans le processus le EUID sera celui du propriétaire.
	#par contre RUID sera celui de la personne qui execute le fichier.
	chmod u+s ./somename
	@echo "Installation reussie"