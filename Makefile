.include "common.mk"

# pkg-config
PKG_CFLAGS != pkg-config --cflags ${PKGS}
PKG_LIBS   != pkg-config --libs   ${PKGS}
CFLAGS += ${PKG_CFLAGS}
LDLIBS += ${PKG_LIBS} -lm

# detect clang
CC_VERSION != ${CC} --version 2>/dev/null || true
.if ${CC_VERSION:M*clang*}
CFLAGS += -fcolor-diagnostics
.endif

