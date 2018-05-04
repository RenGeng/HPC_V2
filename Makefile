all:
	make -C Découpage\ par\ bande/Non_bloquant/Code/
	make -C Découpage\ par\ bande/Bloquant/Code

clean:
	make clean -C Découpage\ par\ bande/Non_bloquant/Code/
	make clean -C Découpage\ par\ bande/Bloquant/Code