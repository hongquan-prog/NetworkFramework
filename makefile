.PHONY:all clean

TARGET := example
DEBUG := true

BINARY_OUTPUT_DIR := build
TARGET := $(addprefix $(BINARY_OUTPUT_DIR)/, $(TARGET))
C_SRC := $(wildcard src/*.c *.c)
C_INC := inc


C_FLAGS := $(addprefix -I, $(C_INC))
CFLAGS += -Wall 

ifneq ("$(DEBUG)","")
	C_FLAGS += -fsanitize=address -g
endif

all: $(BINARY_OUTPUT_DIR) $(TARGET)

$(BINARY_OUTPUT_DIR):
	mkdir -p $@

$(TARGET): $(C_SRC)
	gcc $(C_FLAGS) $^ -o $@

clean:
	rm -r $(BINARY_OUTPUT_DIR)
	

