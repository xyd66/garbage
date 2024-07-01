CC := aarch64-linux-gnu-gcc
SRC := $(shell find src -name "*.c")
INC := ./inc \
	./3rd/usr/local/include \
	./3rd/usr/include \
	./3rd/usr/include/python3.10 \
	./3rd/usr/include/aarch64-linux-gnu \
	./3rd/usr/lib/aarch64-linux-gnu/python3.10 \
	./3rd/usr/include/aarch64-linux-gnu

OBJ :=$(subst src/,obj/,$(SRC:.c=.o))

TARGET = obj/garbage

CFLAGS := $(foreach item,$(INC),-I$(item))# -I./inc -Ixxx
LIBS_PATH := ./3rd/usr/local/lib \
				./3rd/lib/aarch64-linux-gnu \
				./3rd/usr/lib/aarch64-linux-gnu \
				./3rd/usr/lib/python3.10 

LDFLAGS := $(foreach item,$(LIBS_PATH),-L$(item))# ./3rd/usr/local/libs
#LDFLAGS := $(foreach item,$(LIBS_PATH),-L$(item) -Wl,-rpath-link,$(item))
LIBS :=  -lwiringPi -lpython3.10 -lpthread -lexpat -lz -lcrypt

obj/%.o:src/%.c
	mkdir -p obj
	$(CC) -o $@ -c $< $(CFLAGS)

$(TARGET) :$(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

compile: $(TARGET)

clean: 
	rm $(TARGET) obj $(OBJ) -rf

debug:
	echo $(CC)
	echo $(INC)
	echo $(OBJ)
	echo $(TARGET)
	echo $(CFLAGS)
	echo $(LDFLAGS)
	echo $(LIBS)


.PHONY:clean compile debug