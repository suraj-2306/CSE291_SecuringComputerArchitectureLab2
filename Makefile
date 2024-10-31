CC = gcc
CFLAGS = -Wall -Wextra -O0 -g
LDFLAGS = -lm

TARGETS = spectre_attack flush_reload

SPECTRE_SRCS = spectreAttack.c
SPECTRE_OBJS = $(SPECTRE_SRCS:.c=.o)

FLUSH_RELOAD_SRCS = flushReload.c
FLUSH_RELOAD_OBJS = $(FLUSH_RELOAD_SRCS:.c=.o)

HEADER = utility.h

.PHONY: all clean

all: $(TARGETS)

spectre_attack: $(SPECTRE_OBJS) FORCE
	$(CC) $(CFLAGS) -o $@ $(SPECTRE_OBJS) $(LDFLAGS)

flush_reload: $(FLUSH_RELOAD_OBJS) FORCE
	$(CC) $(CFLAGS) -o $@ $(FLUSH_RELOAD_OBJS) $(LDFLAGS)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

FORCE:

clean:
	rm -f $(SPECTRE_OBJS) $(FLUSH_RELOAD_OBJS) $(TARGETS)