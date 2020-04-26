#pacman -S mingw-w64-x86_64-libxml2

TARGET_EXEC ?= collada_loader.dll


BUILD_DIR ?= ../bin
SRC_DIRS ?= ./src ./fcollada/

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.cxx -or -name *.c -or -name *.s | grep -v Test | grep -v FColladaTools)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

LIBS := $(shell find $(SRC_DIRS) -name *.lib) 
INC_DIRS := ./fcollada/FCollada/ ../core/ 
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) $(shell xml2-config --cflags)
INC_LIBS := $(addprefix -L,$(LIBS)) $(shell xml2-config --libs)

CPPFLAGS ?= $(INC_FLAGS) -c -fvisibility=hidden -Wno-unused-parameter -Wno-conversion-null -Wno-unused-function -DUNICODE -DWIN32 -O1 -DNDEBUG -DRETAIL -DIN_LIBXML
# -g -O2 -D __WIN32__ -D WIN32 -D UNICODE -D NDEBUG -D RETAIL -D _STD_AFX_H_ -D FCOLLADA_EXPORT=""
#-shared -Xlinker --add-stdcall-alias -lstdc++
LDFLAGS := $(INC_LIBS) -g -shared -static -lstdc++

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