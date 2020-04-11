#pacman -S mingw-w64-i686-minizip

TARGET_EXEC ?= tv.dll

BUILD_DIR ?= ./bin
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.cxx -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)


LIBS := $(shell find $(SRC_DIRS) -name *.lib) $(shell find $(INC) -name *.lib)
INC_DIRS := $(shell find $(SRC_DIRS) -type d) $(shell find $(INC) -type d) 
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
INC_LIBS := $(addprefix -L,$(LIBS))

CPPFLAGS ?= $(INC_FLAGS) -g -std=c++17 -D __WIN32__
#-shared -Xlinker --add-stdcall-alias 
LDFLAGS := -g -shared -lminizip -lGdi32 -lstdc++ -lopengl32 -lglu32

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) 

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p