MAKE        = make
TARGET1_DIR = wc
TARGET2_DIR = ls
TARGET3_DIR = wc
TARGET1     = $(TARGET1_DIR)/mytail
TARGET2     = $(TARGET2_DIR)/myls
TARGET3     = $(TARGET3_DIR)/mywc


.PHONY : all
all :
	$(MAKE) -C $(TARGET1_DIR)
	$(MAKE) -C $(TARGET2_DIR)
	$(MAKE) -C $(TARGET3_DIR)


.PHONY : clean
clean :
	$(MAKE) -C $(TARGET1_DIR) clean
	$(MAKE) -C $(TARGET2_DIR) clean
	$(MAKE) -C $(TARGET3_DIR) clean

.PHONY : objclean
objclean :
	$(MAKE) -C $(TARGET1_DIR) objclean
	$(MAKE) -C $(TARGET2_DIR) objclean
	$(MAKE) -C $(TARGET3_DIR) objclean
