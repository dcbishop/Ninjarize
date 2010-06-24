all: ninjarize

ninjarize: main.c
	gcc `pkg-config --cflags --libs webkit-1.0 gtk+-2.0` main.c -o ninjarize

clean:
	rm -rf ninjarize 
