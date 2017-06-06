CC := g++
FLAGS := -std=c++14
BUILD_DIR := build
INC_DIR := include
SRC_DIR := src
INCLUDE := -I./$(INC_DIR)

main: $(BUILD_DIR)/test.o $(BUILD_DIR)/BallTree.o \
	$(BUILD_DIR)/BallTreeImpl.o $(BUILD_DIR)/MIPSearcher.o \
	$(BUILD_DIR)/Utility.o
	$(CC) $(FLAGS) $(INCLUDE) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) $(INCLUDE) -c -o $@ $<
  
clean:  
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)
	@rm main