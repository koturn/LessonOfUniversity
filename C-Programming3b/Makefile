MAKE        = make
TARGET1_DIR = calc-game
TARGET2_DIR = tetris
TARGET3_DIR = type-game
TARGET4_DIR = number-game
TARGET1     = $(TARGET1_DIR)/server $(TARGET1_DIR)/client
TARGET2     = $(TARGET2_DIR)/server $(TARGET2_DIR)/client
TARGET3     = $(TARGET3_DIR)/server $(TARGET3_DIR)/client
TARGET4     = $(TARGET4_DIR)/server $(TARGET4_DIR)/client


.PHONY : all
all : $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)

$(TARGET1) :
	$(MAKE) -C $(TARGET1_DIR)

$(TARGET2) :
	$(MAKE) -C $(TARGET2_DIR)

$(TARGET3) :
	$(MAKE) -C $(TARGET3_DIR)

$(TARGET4) :
	$(MAKE) -C $(TARGET4_DIR)


.PHONY : clean
clean :
	$(MAKE) -C $(TARGET1_DIR) clean
	$(MAKE) -C $(TARGET2_DIR) clean
	$(MAKE) -C $(TARGET3_DIR) clean
	$(MAKE) -C $(TARGET4_DIR) clean

.PHONY : objclean
objclean :
	$(MAKE) -C $(TARGET1_DIR) objclean
	$(MAKE) -C $(TARGET2_DIR) objclean
	$(MAKE) -C $(TARGET3_DIR) objclean
	$(MAKE) -C $(TARGET4_DIR) objclean
