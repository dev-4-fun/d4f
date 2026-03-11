SRC_DIR=src
BUILD_DIR=build
INCLUDE_DIR=include
LIB_DIR=lib
LIB_PREFIX=d4f
CC=clang
CFLAGS=-std=c89 -Wall -Wextra -pedantic -glldb

.PHONY: clean array.a array.so array_ho.h

json:
	$(CC) $(CFLAGS) -o json json.c helpers.c

array.o: dirs
	$(CC) $(CFLAGS) -c -o $(BUILD_DIR)/array.o $(SRC_DIR)/array.c -fPIC

$(INCLUDE_DIR)/$(LIB_PREFIX)/array.h:
	cp $(SRC_DIR)/array.h $(INCLUDE_DIR)/$(LIB_PREFIX)/array.h

array.a: array.o $(INCLUDE_DIR)/$(LIB_PREFIX)/array.h
	ar -r $(LIB_DIR)/lib$(LIB_PREFIX)array.a $(BUILD_DIR)/array.o

array.so: array.o $(INCLUDE_DIR)/$(LIB_PREFIX)/array.h
	$(CC) -shared -o $(LIB_DIR)/lib$(LIB_PREFIX)array.so $(BUILD_DIR)/array.o

array_ho.h: dirs
	@echo "/* Header-only version of d4f/array.h */" > $(INCLUDE_DIR)/$(LIB_PREFIX)/array_ho.h
	@cat $(SRC_DIR)/array.h >> $(INCLUDE_DIR)/$(LIB_PREFIX)/array_ho.h
	@echo "\n#ifdef D4F_ARRAY_IMPLEMENTATION" >> $(INCLUDE_DIR)/$(LIB_PREFIX)/array_ho.h
	@cat $(SRC_DIR)/array.c | grep -v "#define D4F_ARRAY_IMPLEMENTATION" | grep -v '#include "array.h"' >> $(INCLUDE_DIR)/$(LIB_PREFIX)/array_ho.h
	@echo "\n#endif" >> $(INCLUDE_DIR)/$(LIB_PREFIX)/array_ho.h

array: array.a array.so array_ho.h

dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(INCLUDE_DIR)/$(LIB_PREFIX)
	mkdir -p $(LIB_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(INCLUDE_DIR)
	rm -rf $(LIB_DIR)

