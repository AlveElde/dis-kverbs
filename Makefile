CC := gcc
CFLAGS := -c -fPIC -Wall -g
LDFLAGS := -shared
RM= rm -f


LIBRARY := libdis-rdmav22.so
SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)
OBJECTS = $(SOURCES:.c=.o)

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(LIBRARY) $(OBJECTS)

clean:
	${RM} ${LIBRARY} ${OBJECTS} $(SOURCES:.c=.d)