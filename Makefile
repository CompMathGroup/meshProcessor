SRC=mesh.cpp
OBJ=$(SRC:.cpp=.o)
TARGET=libmesh3d.a
CXXFLAGS= -O3 -Wall --openmp

.PHONY: all clean

all: $(TARGET)

clean:: 
	rm -f $(TARGET)

$(TARGET) : $(OBJ)
	ar rcs $@ $^

clean::
	rm -f $(OBJ) 
