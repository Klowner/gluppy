gluppyd: gluppyd.c latitude.o
	gcc `pkg-config --cflags --libs gtk+-2.0 gconf-2.0 libcurl liblocation` latitude.o gluppyd.c -o gluppyd

latitude.o: latitude.h latitude.c
	gcc `pkg-config --cflags --libs libcurl` latitude.c -c


clean:
	rm *.o gluppyd
