EXE = cprt

SRCDIR = src
OBJDIR = obj

SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

CFLAGS = -Wall -qopenmp #-fopenmp
CXX = icpc
INCLUDES = -Iinclude
LIBS = -lGL -lglut #-lm

.PHONY: all clean

all: ${EXE}

${EXE}:${OBJ}
	${CXX} ${CFLAGS} ${INCLUDES} -o $@ ${OBJ} ${LIBS}

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) ${INCLUDES} -c $< -o $@

clean:
	-rm -f $(OBJDIR)/*.o core *.core
