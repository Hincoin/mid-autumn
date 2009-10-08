
ROOT_DIR:=../..


include $(ROOT_DIR)/Makefile.include

FILES=$(wildcard *.cpp)
MODULES:=$(FILES:%.cpp=%.o)
INCLUDE:=-I$(ROOT_DIR)/$(EXTERN_INC_DIR) -I$(ROOT_DIR)/$(CORE_INC_DIR) -I$(ROOT_DIR)/$(PBR_INC_DIR) -I$(ROOT_DIR)/$(TBB_INC_DIR)

TARGET:=pbr

#all: $(DEPENDENCIES)
all: $(MODULES)
	@$(CC) $(CC_FLAGS) -o $(TARGET) *.o -L$(ROOT_DIR)/$(BIN_DIR)/$(BUILD_FLAG) $(LINK_LIBS)
	@$(ECHO) $(MSG_COPY)
	@$(CP) $(TARGET) $(ROOT_DIR)/$(BIN_DIR)/$(BUILD_FLAG)

%.d: %.cpp
	@$(CC) -MM $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

sinclude $(MODULES:.o=.d)



clean:
	@$(ECHO) $(MSG_CLEAN) $(INCLUDE)
	@$(RM) -f *.o *.a *.d *.so
	@for i in $(TARGET); do \
		$(RM) -f $(ROOT_DIR)/$(BIN_DIR)/$(BUILD_FLAG)/$$i; \
		done;

