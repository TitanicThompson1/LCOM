PROG=proj

SRCS = proj.c video.c sprite.c keyboard.c utils.c evento.c mouse.c state_machine.c timer.c

# __LCOM_OPTIMIZED__ disables most of the LCF's overhead (and functionality!)
CPPFLAGS += -pedantic #-D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>
