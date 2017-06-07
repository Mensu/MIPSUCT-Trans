CC := g++
FLAGS := -std=c++14
BUILD_DIR := build
INC_DIR := include
SRC_DIR := src
TEST_DIR := test
INCLUDE := -I./$(INC_DIR)

test_main: $(BUILD_DIR)/BallTree.o $(BUILD_DIR)/test-all.o\
	$(BUILD_DIR)/BallTreeImpl.o $(BUILD_DIR)/MIPSearcher.o \
	$(BUILD_DIR)/Utility.o $(BUILD_DIR)/page.o \
	$(BUILD_DIR)/slot.o $(BUILD_DIR)/NodeBuilder.o
	$(CC) $(FLAGS) $(INCLUDE) $^ -o $@

main: $(BUILD_DIR)/test.o $(BUILD_DIR)/BallTree.o \
	$(BUILD_DIR)/BallTreeImpl.o $(BUILD_DIR)/MIPSearcher.o \
	$(BUILD_DIR)/Utility.o $(BUILD_DIR)/page.o \
	$(BUILD_DIR)/slot.o $(BUILD_DIR)/NodeBuilder.o \
	$(BUILD_DIR)/storage.o
	$(CC) $(FLAGS) $(INCLUDE) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) $(INCLUDE) -c -o $@ $<

$(BUILD_DIR)/test-all.o: $(TEST_DIR)/test-all.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(FLAGS) $(INCLUDE) -c -o $@ $<
  
clean:  
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)
	@rm -rf main
	@rm -rf test_main