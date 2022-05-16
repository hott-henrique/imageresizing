CFLAGS := -Wall -Wextra -iquote ./include
DEFS := -DNONE
NAME := tp2

SRC_DIR := src
OBJ_DIR := obj

OBJ := $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c, %.o, $(wildcard $(SRC_DIR)/*.c))))

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) -lm

obj/%.o: $(SRC_DIR)/%.c
	$(CC) $(DEFS) -c $(CFLAGS) $< -o $@

.PHONY: clean help
clean:
	rm -f $(OBJ_DIR)/*.o ./$(NAME)

help:
	@echo "Usages:"
	@echo "	make"
	@echo "	make DEFS=YOUR_OPTIONS_HERE"
	@echo "Options:"
	@echo "	-DSAVE_TEMPS -DSAVE_FREQUENCY=25"
	@echo "	-DSOBEL_FELDMAN"
