#-----------------------COMPILATION------------------------------------------------------
# Compiler and Flags
CC						:= c++
CFLAGS				 	 = -Wall -Wextra -Werror $(INCLUDES) $(CPP_VERSION) -g
TESTS_CFLAGS			 = -Wall -Wextra -Werror $(TESTS_INCLUDES) $(TESTS_CPP_VERSION) -g

# Include paths
INCLUDES				 = $(addprefix -I,$(SRC_DIRS))
TESTS_INCLUDES			 = -I$(TESTS_DIR)/googletest/googletest/include \
							-I$(TESTS_DIR)/googletest/googlemock/include \
							$(INCLUDES)

# C++ versions
CPP_VERSION				 = -std=c++17
TESTS_CPP_VERSION		 = -std=c++14

#-----------------------BINARIES---------------------------------------------------------
# Output Files
NAME					:= webserv
LIBRARY_FOR_TESTS		 = $(TESTS_BIN_FOLDER)/$(NAME).a

#---------TESTS------------
#Tests
TESTS_NAME				 = runtests

#-----------------------FOLDERS----------------------------------------------------------
# Directories
SRC_DIR					:= src
BUILD_DIR				:= build
OBJ_DIR					:= $(BUILD_DIR)/obj
TESTS_DIR				:= tests
TESTS_BIN_FOLDER		:= $(BUILD_DIR)/tests

SRC_DIRS				 = $(patsubst %/, %, $(sort $(dir $(HEADERS))))

#---------TESTS------------
#Tests Directories
TESTS_SRC_DIR			:= $(TESTS_DIR)/src
TESTS_OBJ_DIR			:= $(BUILD_DIR)/$(TESTS_DIR)/obj
GTEST_DIR 				:= $(TESTS_DIR)/googletest

#-----------------------FILES------------------------------------------------------------
# Sources
CPP_FILES 				:= $(shell find $(SRC_DIR) -name '*.cpp')
HEADERS 				:= $(shell find $(SRC_DIR) -name '*.hpp')

# Objects
OBJ     				:= $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP_FILES))

#---------TESTS------------
# Test sources
TEST_CPP_FILES 			:= $(shell find $(TESTS_SRC_DIR) -name '*.cpp')

# Test Objects
TESTS_OBJ				:= $(patsubst $(TESTS_SRC_DIR)/%.cpp, $(TESTS_OBJ_DIR)/%.o, $(TEST_CPP_FILES))

#-------------------------LIBRARIES------------------------------------------------------
# gtest library
GTEST_LIB				:= $(GTEST_DIR)/lib/libgtest.a \
							$(GTEST_DIR)/lib/libgtest_main.a

#-----------------------COLORS-----------------------------------------------------------
# Colors for Output
GREEN					:= \033[0;32m
RED						:= \033[31m
BLUE					:= \033[0;34m
YELLOW					:= \033[0;33m
RESET					:= \033[0m

#-----------------------RULES------------------------------------------------------------
# Default Target
all: $(NAME)

# Build the Executable
$(NAME): $(OBJ)
	@$(CC) $(CFLAGS) $^ -o $@
	@echo "$(GREEN)Compiled $@ successfully!$(RESET)"

# Compile Object Files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) $(TEMPLATES) Makefile
	$(if $(COMPILE_MSG_SHOWN),,$(eval COMPILE_MSG_SHOWN := 1) \
	@echo "$(YELLOW)>> Compiling object files...$(RESET)")
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Collect all .o files into .a file to use it for unit tests
$(LIBRARY_FOR_TESTS): $(OBJ)
	@ar src $@ $^
	@ar d $@ main.o

#---------TESTS------------
# Make tests
tests: $(TESTS_NAME)

# Build tests
$(TESTS_NAME): $(TESTS_OBJ) $(LIBRARY_FOR_TESTS) $(GTEST_LIB)
	@mkdir -p $(dir $@)
	@$(CC) $(TESTS_CFLAGS) $(TESTS_OBJ) $(LIBRARY_FOR_TESTS) $(GTEST_LIB) $(LDFLAGS) -o $@
	@echo "$(GREEN)Compiled $@ successfully!$(RESET)"

# Compile test object files
$(TESTS_OBJ_DIR)/%.o: $(TESTS_SRC_DIR)/%.cpp $(HEADERS) $(TEMPLATES) Makefile
	$(if $(TESTS_COMPILE_MSG_SHOWN),,$(eval TESTS_COMPILE_MSG_SHOWN := 1) \
	@echo "$(YELLOW)>> Compiling tests...$(RESET)")
	@mkdir -p $(dir $@)
	@$(CC) $(TESTS_CFLAGS) -c $< -o $@
#-----------END------------

# Clean up Object Files
clean:
	@rm -rf $(BUILD_DIR)
	@echo "$(RED)Removed object files$(RESET)"

# Clean up All Generated Files
fclean: clean
	@rm -rf $(NAME)
	@rm -rf $(TESTS_NAME)
	@echo "$(RED)Removed $(NAME)$(RESET)"

# Rebuild the Project
re: fclean all

# Phony Targets
.PHONY: all clean fclean re tests
