#++++++++++COLORS+++++++++++
RED	:=	\033[0;31m
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE :=\033[0;34m
RESET :=\033[0;35m

#++++++++++PROGRAM NAMES+++++++++++
NAME	=	codexion

#++++++++++COMPILER+++++++++++
CC	= 	cc
CFLAGS	 =	-Wall -Wextra -pthread -I includes
RM	:=	rm -f
RMDIR := rm -rf
SRCS 	:= 	src/main.c src/parser.c src/init.c src/utils.c src/coder.c src/dongle.c src/eye.c
OBJS := $(SRCS:.c=.o)

#++++++++++RULES+++++++++++
.PHONY: all clean fclean re help

al: $(NAME)

$(NAME): $(OBJS)
	$(CC)	$(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cc
	$(CC)	$(CFLAG)	-c $<	-o	$@


#++++++++++CLEAN+++++++++++
clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@$(RM) $(OBJS)
	@echo "$(GREEN)✓ Everything cleaned!$(RESET)"
	
fclean: clean
	@echo "$(RED)Removing executables...$(RESET)"
	@$(RM) $(NAME)
	@echo "$(GREEN)✓ Everything cleaned!$(RESET)"

re: fclean all
