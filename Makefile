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
CFLAGS := -Werror -Wall -Wextra -I include -std=c89 -O3

BUILD_MODE ?= release

ifeq ($(BUILD_MODE), dev)
    CFLAGS += -g -D_DEV
else ifeq ($(BUILD_MODE), release)
    CFLAGS +=
endif

.PHONY: all help install uninstall clean dev release

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
	rm -rf $(CONFDIR)/

clean:
	rm -f $(OBJ) $(PKG_NAME)

dev: 
	$(MAKE) BUILD_MODE=dev

release:
	$(MAKE) BUILD_MODE=release

help:
	@echo "Usage: make [target] [BUILD_MODE=mode]"
	@echo ""
	@echo "Targets:"
	@echo "  all        Build the package"
	@echo "  dev        Build with debug info and no optimizations"
	@echo "  release    Build with full optimization and no debug info"
	@echo "  install    Install piratpkg"
	@echo "  uninstall  Uninstall piratpkg"
	@echo "  clean      Clean build files"
	@echo "  help       Display this help message"
