CC = cc
CFLAGS = -std=c99 -Wall -Wextra -O2
CPPFLAGS = -Iinclude -D_POSIX_C_SOURCE=200809L

SRCS = source/wsxwm.c \
       source/util.c
HDRS = include/wsxwm.h \
       include/util.h
OUT = wsxwm

PKGS = swc wayland-server xkbcommon libinput pixman-1 libdrm \
       wld libudev xcb xcb-composite xcb-ewmh xcb-icccm
PKG_CFLAGS != pkg-config --cflags ${PKGS}
PKG_LIBS   != pkg-config --libs   ${PKGS}
CFLAGS += ${PKG_CFLAGS}
LDLIBS += ${PKG_LIBS} -lm

all: $(OUT)
$(OUT): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $(OUT) $(SRCS) $(LDLIBS)

clean:
	rm -f $(OUT)

compile_flags:
	rm -f compile_flags.txt
	for f in $(CFLAGS) $(CPPFLAGS); do echo $$f >> compile_flags.txt; done


