CFLAGS=-Wall -Wextra -O3 -g

all: aiken decode decoded.txt ticket

aiken: aiken.c translink.h
	cc aiken.c -o aiken -lsndfile $(CFLAGS)

decode: translink.h decode.c
	cc decode.c -o decode $(CFLAGS)

decoded.txt: tickets.txt decode
	cat tickets.txt | while read line; do echo -ne "`echo $$line | cut -d' ' -f 1`\t"; ./decode "`echo $$line | cut -d' ' -f 2`"; done > decoded.txt

ticket: ticket.cpp translink.h
	convert ticket-small.png ticket-small.xpm
	convert font.png font.xpm

	sed -i 's/static char/static const char/' ticket-small.xpm
	sed -i 's/static char/static const char/' font.xpm

	sed -i 's/ticket_small/ticket_small_xpm/' ticket-small.xpm
	sed -i 's/font/font_xpm/' font.xpm

	g++ -c 'ticket.cpp' -o 'ticket.o' $(shell fltk-config --cxxflags)
	g++ 'ticket.o' -o 'ticket' $(shell fltk-config --ldstaticflags)
