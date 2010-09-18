TARGET = f8test
LIBS = -lrt -lf8 -lPocoFoundation
INCLUDE = -I../include -I.
CXX_SRCS = myfix.cpp myfix_types.cpp myfix_classes.cpp
GENERATED_SRC = myfix_types.cpp myfix_classes.cpp myfix_types.hpp myfix_classes.hpp
CXXFLAGS = -O3 -ggdb -Wall
LDFLAGS = -ggdb -L../lib
XML_SCHEMA = FIX44TEST.xml
OBJS = $(CXX_SRCS:.cpp=.o)

%.o: %.cpp
	$(CXX) -c $(INCLUDE) $(CXXFLAGS) $< -o $@

$(TARGET) : $(OBJS) $(GENERATED_SRC)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

$(GENERATED_SRC): FIX44TEST.xml

clean:
	-rm -f $(OBJS) core $(TARGET) $(GENERATED_SRC)

depend:
	$(CXX) -MM $(CXX_SRCS)

