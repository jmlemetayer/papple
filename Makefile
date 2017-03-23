# Disable built-in rules and variables
MAKEFLAGS	+= -rR

# Set default CC value
ifeq ($(origin CC),default)
CC		= gcc
endif

# Set default RM value
ifeq ($(origin RM),default)
RM		= rm -f
endif

# Set other default values
CFLAGS		?= -Wall -Wextra -Werror
CPPFLAGS	?=
LDFLAGS		?=
LDLIBS		?=

# Default target
all: papple

papple: main.o logger.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Manage object with dependencies
%.o: %.c
	$(CC) -MMD -MP $(CFLAGS) $(CPPFLAGS) -c $< -o $@

-include *.d

# Cleaning targets
clean:
	$(RM) *.o *.d

mrproper: clean
	$(RM) papple

# Declare PHONY targets
.PHONY: all clean mrproper
