all: ninjarize

ninjarize: main.c
	gcc `pkg-config --cflags --libs webkit-1.0 gtk+` main.c -o ninjarize

clean:
	rm -rf ninjarize 
