# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/28 17:42:26 by sebasnadu         #+#    #+#              #
#    Updated: 2024/08/29 15:42:20 by migmanu          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

################################################################################
##                                   COLORS                                   ##
################################################################################

DEFAULT     := \033[0;39m
GRAY        := \033[0;90m
RED         := \033[0;91m
GREEN       := \033[0;92m
YELLOW      := \033[0;93m
BLUE        := \033[0;94m
MAGENTA     := \033[0;95m
CYAN        := \033[0;96m
WHITE       := \033[0;97m
	
################################################################################
##                                  WEBSERV                                   ##
################################################################################

# Add here the name of the file that contain an specific group of tests.
TESTS							:= ServerInput HttpRequest RequestParser
CXX								:= c++
RM								:= rm -rf
	
################################################################################
##                                DIRECTORIES                                 ##
################################################################################

OBJ_DIR						:= ../obj
SRC_DIR						:= ../srcs
INC_DIRS					:= ./ ../include

vpath %.cpp $(SRC_DIR)
vpath %.hpp $(INC_DIRS)
vpath %.o $(OBJ_DIR)

SOURCE						:= $(shell find $(SRC_DIR) -name '*.cpp' -exec basename {} \;)
SOURCE						:= $(filter-out main.cpp, $(SOURCE))
OBJECTS						:= $(addprefix $(OBJ_DIR)/, $(SOURCE:.cpp=.o))
	
################################################################################
##                                   FLAGS                                    ##
################################################################################

CXXFLAGS						:= -std=c++11
INCLUDE							:= $(addprefix -I, $(INC_DIRS))
TLIB								:= -I/opt/homebrew/include -L/opt/homebrew/lib -lcriterion

define run
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ $(TLIB) -o $@ && ./$@ --verbose
endef

################################################################################
##                                COMPILATION                                 ##
################################################################################

all: $(TESTS) clean

# For each test file you need to create a new rule with an descriptive and no so
# long name, also to have the option to run only this test separately.
.PHONY: ServerInput
ServerInput: $(OBJECTS) ServerInputTest.cpp
	@$(call run, "$^")

.PHONY: HttpRequest
HttpRequest: $(OBJECTS) HttpRequestTest.cpp
	@$(call run, "$^")

.PHONY: RequestParser
RequestParser: $(OBJECTS) RequestParserTest.cpp
	@$(call run, "$^")

$(OBJECTS):
	@make -C .. -s

clean:
	@printf "$(MAGENTA)[tests] $(DEFAULT)Cleaning up tests..."
	@$(RM) $(TESTS)
	@printf "\r%100s\r$(MAGENTA)[tests] $(YELLOW)Full clean success "
	@printf "🧹🧹$(DEFAULT)\n"

fclean: clean

re: fclean all

.PHONY: all clean fclean re
