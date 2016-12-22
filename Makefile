all: go99

go99: src/go99.cpp src/Node.h src/String.h
	g++ src/go99.cpp -o bin/go99 -O3

play:
	sudo bash ./gogui-1.4.9/bin/gogui

terminal:
	sudo bash ./gogui-1.4.9/bin/gogui-terminal bin/go99 -size 9

clean:
	rm bin/*
