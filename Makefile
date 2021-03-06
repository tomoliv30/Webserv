NAME		=	webserv
FLAGS		=	-Wall -Wextra -Werror -std=c++98 -pedantic

ifdef DEBUG
FLAGS+= -g -fsanitize=address
endif

LOG_FILE=

ifdef LOG
LOG_FILE+=  2>&1 | tee webserv.log
endif

INCLUDES	=	-Iincludes

SRC_DIR		= 	srcs

SRCS=	$(SRC_DIR)/main.cpp \
				$(SRC_DIR)/utils.cpp \
				$(SRC_DIR)/Server.cpp \
				$(SRC_DIR)/VirtualServer.cpp \
				$(SRC_DIR)/ServerRun.cpp \
				$(SRC_DIR)/ServerReceive.cpp \
				$(SRC_DIR)/ServerInit.cpp \
				$(SRC_DIR)/ServerHandleRequest.cpp \
				$(SRC_DIR)/ServerAccept.cpp \
				$(SRC_DIR)/ServerRespond.cpp \
				$(SRC_DIR)/Client.cpp \

OBJ			= $(SRCS:.cpp=.o)

CC 			= clang++

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(FLAGS) $(OBJ) -g -o $(NAME)


%.o: %.cpp $(HEADER)
	@$(CC) $(FLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: 
	$(MAKE) fclean
	$(MAKE) all

run: re
		./$(NAME) $(LOG_FILE)

.PHONY: all clean fclean re
