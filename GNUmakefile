include common.mk

# pkg-config
PKG_CFLAGS = $(shell pkg-config --cflags $(PKGS))
PKG_LIBS   = $(shell pkg-config --libs   $(PKGS))
CFLAGS += $(PKG_CFLAGS)
LDLIBS += $(PKG_LIBS) -lm

# detect clang
CC_VERSION := $(shell $(CC) --version 2>/dev/null || true)
ifneq (,$(findstring clang,$(CC_VERSION)))
CFLAGS += -fcolor-diagnostics
endif

