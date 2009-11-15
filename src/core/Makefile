
ROOT_DIR:=../..


include $(ROOT_DIR)/Makefile.include

FILES=$(wildcard *.cpp)
MODULES:=$(FILES:%.cpp=%.o)
INCLUDE:=-I$(ROOT_DIR)/$(EXTERN_INC_DIR) -I$(ROOT_DIR)/$(CORE_INC_DIR) -I$(ROOT_DIR)/$(TBB_INC_DIR)
OBJ_DIR:=.

TARGET:=core_test

#all: $(DEPENDENCIES)
all: $(MODULES) 
	@$(CC) $(CC_FLAGS) -o $(TARGET) $(OBJ_DIR)/*.o -L$(ROOT_DIR)/$(BIN_DIR)/$(BUILD_FLAG) $(LINK_LIBS)
	@$(ECHO) $(MSG_COPY)
	@$(MV) $(TARGET) $(ROOT_DIR)/$(BIN_DIR)/$(BUILD_FLAG)
	@$(RM) -f *.d

%.d: %.cpp
	@$(CC) -MM $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	@$(RM) -f $@.$$$$

sinclude $(MODULES:.o=.d)



clean:
	@$(ECHO) $(MSG_CLEAN) $(INCLUDE)
	@$(RM) -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.a *.d $(OBJ_DIR)/*.so
	@for i in $(TARGET); do \
		$(RM) -f $(ROOT_DIR)/$(BIN_DIR)/$(BUILD_FLAG)/$$i; \
		done;

