PKG_NAME := piratpkg
PKG_VERSION := 1.0
PKG_DESC := Minimal package manager
PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include
CONFDIR := /etc/piratpkg

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
CFLAGS := -Werror -Wall -Wextra -O2 -I include -std=c89

.PHONY: all help install uninstall clean

all: $(PKG_NAME)

$(PKG_NAME): $(OBJ)
	$(CC) $(OBJ) -o $(PKG_NAME)

install: $(PKG_NAME)
	@echo "Installing piratpkg to $(PREFIX)"
	install -m 755 $(PKG_NAME) $(BINDIR)/$(PKG_NAME)
	@echo "Installing piratpkg.conf to $(CONFDIR)"
	mkdir -p $(CONFDIR)
	install -m 644 piratpkg.conf $(CONFDIR)/piratpkg.conf

uninstall:
	@echo "Uninstalling piratpkg"
	rm -f $(BINDIR)/$(PKG_NAME)
	rm -f $(CONFDIR)/piratpkg.conf

clean:
	rm -f $(OBJ) $(PKG_NAME)

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all        Build the package"
	@echo "  install    Install piratpkg"
	@echo "  uninstall  Uninstall piratpkg"
	@echo "  clean      Clean build files"
	@echo "  help       Display this help message"
