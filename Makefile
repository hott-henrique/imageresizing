CFLAGS := -Wall -Wextra -iquote ./include
DEFS := NONE
NAME := tp2

SRC_DIR := src
OBJ_DIR := obj

OBJ := $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c, %.o, $(wildcard $(SRC_DIR)/*.c))))

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

obj/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -D $(DEFS) -o $@

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o ./$(NAME)
