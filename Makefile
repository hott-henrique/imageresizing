CFLAGS := -Wall -Wextra -iquote ./include
DEFS := -D NONE
NAME := tp2

SRC_DIR := src
OBJ_DIR := obj

OBJ := $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c, %.o, $(wildcard $(SRC_DIR)/*.c))))

$(NAME): obj_dir_check $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) -lm

obj/%.o: $(SRC_DIR)/%.c
	$(CC) $(DEFS) -c $(CFLAGS) $< -o $@

obj_dir_check:
	mkdir -p obj

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o ./$(NAME)
