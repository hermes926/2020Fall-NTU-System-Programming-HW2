all: player.c host.c
	gcc player.c -o player
	gcc host.c -o host

