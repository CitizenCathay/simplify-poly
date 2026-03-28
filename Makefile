CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
TARGET   = simplify

SRCS = src/main.cpp \
       src/polygon.cpp \
       src/collapse.cpp \
       src/intersect.cpp \
       src/spatial_index.cpp \
       src/simplify.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)