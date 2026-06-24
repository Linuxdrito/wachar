CC = gcc
CXX = g++
CFLAGS = -O3
CXXFLAGS = -std=c++17 -Wall -O3 -march=native -flto -Wno-attributes
LIBS = -lwayland-client -lwayland-egl -lEGL -lGLESv2
INCLUDES = -I. -Isrc

PROTO_DIR = /usr/share/wayland-protocols/stable/xdg-shell
PROTO_XML = $(PROTO_DIR)/xdg-shell.xml

all: wachar

xdg-shell-protocol.h:
	wayland-scanner client-header $(PROTO_XML) $@

xdg-shell-protocol.c:
	wayland-scanner private-code $(PROTO_XML) $@

# Compilar el protocolo C nativamente con gcc
xdg-shell-protocol.o: xdg-shell-protocol.c xdg-shell-protocol.h
	$(CC) $(CFLAGS) -c $< -o $@

# Enlazar el objeto C (.o) con el resto de tu código C++
wachar: xdg-shell-protocol.o src/image.cpp src/renderer.cpp src/wayland.cpp src/main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ xdg-shell-protocol.o src/*.cpp $(LIBS)

clean:
	rm -f wachar xdg-shell-protocol.h xdg-shell-protocol.c xdg-shell-protocol.o
