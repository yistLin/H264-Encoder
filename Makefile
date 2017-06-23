COMPILER = $(CXX)

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./
INC_DIR = ./inc

TARGET = $(BIN_DIR)/encoder

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.cpp=.o)))

LIBS =
INCLUDE = -I$(INC_DIR)
CPPFLAGS += -Wall -std=c++1z
LDFLAGS +=
LDLIBS +=

.PHONY: all clean

all: clean $(TARGET)

$(TARGET): $(OBJS)
	$(COMPILER) $(LDFLAGS) $(LDLIBS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(COMPILER) $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	$(RM) $(OBJS)

run:
	$(TARGET)
