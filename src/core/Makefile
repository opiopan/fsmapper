TARGET		 = libmappercore.a
BUILD_DIR	 = build

CXXSOURCES	 = mappercore.cpp\
		   engine.cpp \
		   action.cpp \
		   device.cpp \
		   devicemodifier.cpp \
		   pluginapi.cpp \
		   builtinDevices/simhid.cpp \
		   builtinDevices/simhidconnection.cpp \
		   builtinDevices/posixserial.cpp

CSOURCES	 = builtinDevices/simhidparser.c

INCLUDES	 = -I../../modules/sol2/include \
		   -I../../modules/lua-5.4.3/src \
		   -IbuiltinDevices \
		   -I.

CXXFLAGS	+= -std=c++17 -g -O $(INCLUDES)
CXXFLAGS	+= -MMD -MP -MF"$(@:%.o=%.d)"
CFLAGS		+= -g -O $(INCLUDES)
CFLAGS		+= -MMD -MP -MF"$(@:%.o=%.d)"

all: $(BUILD_DIR)/$(TARGET)


OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(CXXSOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CXXSOURCES)))
OBJS += $(addprefix $(BUILD_DIR)/,$(notdir $(CSOURCES:.c=.o)))
vpath %.c $(sort $(dir $(CSOURCES)))

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET): $(OBJS) Makefile
	$(AR) -rcs $@ $(OBJS)
	
$(BUILD_DIR):
	mkdir $@		

clean:
	rm -rf $(BUILD_DIR)

-include $(wildcard $(BUILD_DIR)/*.d)
