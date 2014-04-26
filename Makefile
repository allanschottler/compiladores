C_FILES := $(wildcard *.c)
OBJ_FILES := $(addprefix ./,$(notdir $(C_FILES:.c=.o)))


mini0: $(OBJ_FILES) ; gcc -g -o $@ $^

obj/%.o: src/%.c ; gcc -g -c -o $@ $<

clean: ; rm -f mini0 *.o
