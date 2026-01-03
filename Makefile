CC = clang
CFLAGS = -Wall -Wextra -g
LIBS = -lX11 -lXcomposite -lXi -lXrandr -lXfixes -lcairo

# Build Configuration

SRCDIR = src
OBJDIR = obj
BINDIR = bin

TARGET = $(BINDIR)/limeos-window-manager

SOURCES = $(shell find $(SRCDIR) -name '*.c')
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

INCLUDES = $(shell find $(SRCDIR) -type d -exec printf "-I{} " \;)
CFLAGS += $(INCLUDES)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Tests Configuration

TESTDIR = tests/unit
TESTOBJDIR = obj/tests
TESTBINDIR = bin/tests

TEST_SOURCES = $(shell find $(TESTDIR) -name '*.c')
TEST_OBJS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(TESTOBJDIR)/%.o)
TEST_BINARIES = $(TEST_SOURCES:$(TESTDIR)/%.c=$(TESTBINDIR)/%)
SRC_OBJECTS = $(filter-out $(OBJDIR)/main.o,$(OBJECTS))

TEST_CFLAGS = -Wall -Wextra -g $(INCLUDES) -Itests
TEST_LIBS = $(LIBS) -lcmocka

$(TESTOBJDIR)/%.o: $(TESTDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(TESTBINDIR)/%: $(TESTOBJDIR)/%.o $(SRC_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $< $(SRC_OBJECTS) -o $@ $(TEST_LIBS)

test: $(TEST_BINARIES)
	@failed=0; \
	for t in $(TEST_BINARIES); do \
		echo ""; \
		echo "Running tests from \"$(notdir $(TESTDIR))/$${t#$(TESTBINDIR)/}\":"; \
		echo ""; \
		$$t || failed=1; \
	done; \
	echo ""; \
	if [ $$failed -eq 0 ]; then \
		echo "All tests passed."; \
	else \
		echo "Some tests failed."; \
		exit 1; \
	fi

test-clean:
	rm -rf $(TESTOBJDIR) $(TESTBINDIR)

# Special Directives

.PRECIOUS: $(TEST_OBJS)
.PHONY: all clean test test-clean
