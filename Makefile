CFLAGS=-Wall -Wextra -O3 -g

all: aiken decode decoded.txt ticket

aiken: aiken.c translink.h
	cc aiken.c -o aiken -lsndfile $(CFLAGS)

decode: translink.h decode.c
	cc decode.c -o decode $(CFLAGS)

decoded.txt: tickets.txt decode
	cat tickets.txt | while read line; do echo -ne "`echo $$line | cut -d' ' -f 1`\t"; ./decode "`echo $$line | cut -d' ' -f 2`"; done > decoded.txt

ticket: ticket.cpp translink.h
	g++ -o 'ticket' 'ticket.cpp' -lpng -ljpeg -lfltk -lfltk_images
