TARGET = ../bin/f8c
LIBS = -lrt -lf8
INCLUDE = -I../include -I.
CXX_SRCS = f8c.cpp
HDRS = f8cstatic.hpp f8c.hpp
CXXFLAGS = -O3 -ggdb -Wall
LDFLAGS = -ggdb -L../lib
OBJS = $(CXX_SRCS:.cpp=.o)

%.o: %.cpp $(HDRS)
	$(CXX) -c $(INCLUDE) $(CXXFLAGS) $< -o $@

$(TARGET) : $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

clean:
	-rm -f $(OBJS) core $(TARGET)

depend:
	$(CXX) -MM $(CXX_SRCS)

