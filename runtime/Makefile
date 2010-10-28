TARGET = ../lib/libf8.so
INCLUDE = ../include
CXX_SRCS = xml.cpp f8utils.cpp message.cpp traits.cpp field.cpp session.cpp logger.cpp
CXXFLAGS = -O3 -ggdb -Wall
LDFLAGS = -ggdb -rdynamic -shared
OBJS = $(CXX_SRCS:.cpp=.o)

%.o: %.cpp ../include/%.hpp
	$(CXX) -c -I$(INCLUDE) $(CXXFLAGS) $< -o $@

$(TARGET) : $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

clean:
	-rm -f $(OBJS) core $(TARGET)

depend:
	$(CXX) -MM $(CXX_SRCS)


