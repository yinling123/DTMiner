# Makefile for DYNAMO command line subgraph search
# Patrick Mackey, 2018 (Modified to skip intermediate files)

SRC = $(wildcard *.cpp)
TARGET = graph_search
# CFLAGS = --std=c++17 -O3 -g -I/home/chenh/hyb/software/numactl-2.0.17-soft/include -fsanitize=address
CFLAGS = --std=c++17 -O3 -g -I/home/chenh/hyb/software/numactl-2.0.17-soft/include

# 链接器选项（仅影响链接阶段）
LDFLAGS = -L/home/chenh/hyb/software/numactl-2.0.17-soft/lib \
          -Wl,-rpath=/home/chenh/hyb/software/numactl-2.0.17-soft/lib

# 库链接选项（放在最后）
LDLIBS = -lnuma

CXX = g++-9

# 禁用所有内置隐式规则和变量
MAKEFLAGS += -rR

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS) -pthread

clean:
	rm -f $(TARGET)
