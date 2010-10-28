TARGET = f8test
LIBS = -lrt -lf8 -lPocoFoundation -ltbb
INCLUDE = -I../include -I.
CXX_SRCS = myfix.cpp Myfix_types.cpp Myfix_classes.cpp Myfix_traits.cpp
GENERATED_SRC = Myfix_types.hpp Myfix_types.cpp Myfix_traits.cpp Myfix_router.hpp Myfix_classes.hpp Myfix_classes.cpp
CXXFLAGS = -O3 -ggdb -Wall
LDFLAGS = -ggdb -L../lib
XML_SCHEMA = FIX44.xml
OBJS = $(CXX_SRCS:.cpp=.o)

%.o: %.cpp
	$(CXX) -c $(INCLUDE) $(CXXFLAGS) $< -o $@

$(TARGET) : $(OBJS) $(GENERATED_SRC)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

gen:
	../bin/f8c $(XML_SCHEMA)

clean:
	-rm -f $(OBJS) core $(TARGET) $(GENERATED_SRC)

depend:
	$(CXX) -MM $(CXX_SRCS)

