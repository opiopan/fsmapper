TARGET		 = testmock
TARGET2		 = testmock_learn_sol
BUILD_DIR	 = build

CXXSOURCES	 = testmock.cpp \
                   testmock_learn_sol.cpp

CSOURCES	 = 

INCLUDES	 = -I../core \
		   -I../../modules/sol2/include \
		   -I../../modules/lua-5.4.3/src
LIBDIRS		 = -L../core/build \
		   -L../../modules/lua-5.4.3/src
LIBS		 = -lmappercore -llua
CORELIB		 = ../core/build/libmappercore.a

CXXFLAGS	+= -std=c++17 -g -O $(INCLUDES)
CXXFLAGS	+= -MMD -MP -MF"$(@:%.o=%.d)"
CFLAGS		+= -g -O $(INCLUDES)
CFLAGS		+= -MMD -MP -MF"$(@:%.o=%.d)
LFLAGS		+= $(LIBDIRS) $(LIBS)

all: core $(BUILD_DIR)/$(TARGET)

all2: all $(BUILD_DIR)/$(TARGET2)

OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(CXXSOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CXXSOURCES)))
OBJS += $(addprefix $(BUILD_DIR)/,$(notdir $(CSOURCES:.c=.o)))
vpath %.c $(sort $(dir $(CSOURCES)))

$(BUILD_DIR)/$(TARGET): $(CORELIB) $(BUILD_DIR)/testmock.o Makefile
	$(CXX) $(LFLAGS) -o $@ $(BUILD_DIR)/testmock.o

$(BUILD_DIR)/$(TARGET2): $(CORELIB) $(BUILD_DIR)/testmock_learn_sol.o Makefile
	$(CXX) $(LFLAGS) -o $@ $(BUILD_DIR)/testmock_learn_sol.o

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir $@		

$(CORELIB): core

core:
	make -C../core

clean:
	make -C../core clean
	rm -rf $(BUILD_DIR)

-include $(wildcard $(BUILD_DIR)/*.d)
