CC := g++
CPPFLAGS := -std=c++11 -g
TARGET := afin
OBJS := afin.o contig.o print_time.o process.o read.o revcomp.o

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $(OBJS)

%.o : %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@ 

clean:
	/bin/rm -f *o $(TARGET)

