SHELL  = /bin/sh

CFLAGS      = -Wall -Wextra -Wshadow -Wpointer-arith -Wstrict-prototypes \
              -Wdeclaration-after-statement -Wcast-qual -g
LDFLAGS     = -g
LDLIBS      =

COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

# Linux code style
# See https://www.gnu.org/software/indent/manual/indent.html
INDENTFLAGS = -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 \
              -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs -nprs -npsl -sai \
              -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1

SRCDIR = .
OBJDIR = obj
SRCS   = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*/*.c)
OBJS   = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

TARGET  = object-demo

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(SRCDIR)/*.c~

distclean:
	$(RM) $(TARGET)

indent:
	indent $(INDENTFLAGS) \
		$(SRCDIR)/*.c

$(OBJDIR):
	@mkdir -p $@
