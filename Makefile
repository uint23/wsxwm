CC      ?= cc
CFLAGS   = -std=c99 -Wall -Wextra -O2 -fcolor-diagnostics
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -Isource/include

OUT = wsxwm
SRC = source/wsxwm.c

PKGS = swc wayland-server xkbcommon libinput pixman-1 libdrm wld libudev xcb xcb-composite xcb-ewmh xcb-icccm
CFLAGS += $(shell pkg-config --cflags $(PKGS))
LDLIBS += $(shell pkg-config --libs   $(PKGS)) -lm

all: ${OUT}

${OUT}: ${SRC}
	${CC} ${CFLAGS} ${CPPFLAGS} -o ${OUT} ${SRC} ${LDLIBS}

clean:
	rm -f ${OUT}

compile_flags:
	rm -f compile_flags.txt
	for f in ${CFLAGS} ${CPPFLAGS}; do echo $$f >> compile_flags.txt; done

