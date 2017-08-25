CC=g++
BIN=demo

CPPFLAGS := -Wall -Wextra -std=c++11
CPPFLAGS += -g


SRCFILES := main.cpp
SOURCES := $(addprefix src/, $(SRCFILES))

OBJFILES := $(patsubst %.cpp,%.o, $(SRCFILES))
OBJS := $(addprefix build/, $(OBJFILES))

all: $(BIN)

$(BIN): build $(OBJS)
	@echo "LD   $(BIN)"
	@$(CC) -lSDL $(OBJS) -o $(BIN)

build:
	@echo "DIR  build"
	@mkdir -p build/


build/%.o: src/%.cpp
	@echo "CC   $<"
	@$(CC) -c $(CPPFLAGS) -o $@ $<

clean:
	@rm -rf build
	@rm -f $(BIN)

.PHONY: clean
