CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++17
TARGET = server

SRCS = server.cpp TcpServer.cpp EventLoop.cpp Connection.cpp Acceptor.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)