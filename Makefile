###
### con Makefile
### ------------
###

### Common
### ------
TRG      = con
SYS      = $(shell uname)
OBJ_DIR  = OBJ_$(SYS)

### General flags
ifdef DEBUG
CFLAGS = -g -Wall -fno-exceptions -W -Werror
CPPFLAGS = -g -Wall -fno-exceptions -W -Werror
else
CFLAGS   = -O5 -Wall -fno-exceptions -W -Werror
CPPFLAGS = -O5 -Wall -fno-exceptions -W -Werror
endif
LFLAGS   =
LIBS     =
CC      = gcc
CLINK   = gcc
CPP     = g++
CPPLINK = g++
DEFS    = -DHOST_X86

all: $(TRG)

clean:
	@rm -fr $(OBJ_DIR) OBJ_$(SYS)_x86 valgrind* *.gdb *.o *.d *.obj $(TRG) *~ html doxy.*


### Input files
### -----------
SRCS   = con.cpp tty.cpp

OBJS  = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

### Load dependecies
### ----------------
DEPS = $(wildcard $(OBJ_DIR)/*.d)
ifneq ($(strip $(DEPS)),)
include $(DEPS)
endif

### Dependecies generation
### --------------------------------------
define DEPENDENCIES
@if [ ! -f $(@D)/$(<F:.c=.d) ]; then \
    sed 's/^$(@F):/$(@D)\/$(@F):/g' < $(<F:.c=.d) > $(@D)/$(<F:.c=.d); \
    rm -f $(<F:.c=.d); \
fi
endef
define DEPENDENCIES_CPP
@if [ ! -f $(@D)/$(<F:.cpp=.d) ]; then \
    sed 's/^$(@F):/$(@D)\/$(@F):/g' < $(<F:.cpp=.d) > $(@D)/$(<F:.cpp=.d); \
    rm -f $(<F:.cpp=.d); \
fi
endef

### Target rules
### ------------
# general compilation
$(OBJ_DIR)/%.o: %.c
		$(CC) -c -MD $(CFLAGS) $(INC) $(DEFS) -o $@ $<
		$(DEPENDENCIES)
$(OBJ_DIR)/%.o: %.cpp
		$(CPP) -c -MD $(CPPFLAGS) $(INC) $(DEFS) -o $@ $<
		$(DEPENDENCIES_CPP)

$(OBJ_DIR):
		mkdir $(OBJ_DIR)

# Targets
$(TRG):	$(OBJ_DIR) $(OBJS)  Makefile
		$(CPPLINK) -o $@ $(LFLAGS) $(OBJS) $(LIBS)
