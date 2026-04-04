SRC_DIR=src
BUILD_DIR=build
TEST_DIR=tests
INCLUDE_DIR=include
LIB_DIR=lib
LIB_PREFIX=d4f
CC=clang
CTAGS?=ctags

BUILD ?= debug
ifeq ($(BUILD),release)
  CFLAGS=-std=c89 -Wall -Wextra -pedantic -O3
else
  CFLAGS=-std=c89 -Wall -Wextra -Werror -pedantic -glldb
endif

.PHONY:\
	clean\
	tags\
	test\
	release\
	array.a\
	array.so\
	array_ho.h\
	array_view.a\
	array_view.so\
	array_view_ho.h

release:
	$(MAKE) BUILD=release array
	$(MAKE) BUILD=release array_view

test: array array_view
	$(BUILD_DIR)/array.test
	$(BUILD_DIR)/array_view.test

array_view.o: dirs
	$(CC) $(CFLAGS) -c -o $(BUILD_DIR)/$@ -I$(INCLUDE_DIR) $(SRC_DIR)/$(subst .o,.c,$@) -fPIC

array_view.a: array_view.o
	ar -r $(LIB_DIR)/lib$(LIB_PREFIX)$@ $(BUILD_DIR)/$<

array_view.so: array_view.o
	$(CC) -shared -o $(LIB_DIR)/lib$(LIB_PREFIX)$@ $(BUILD_DIR)/$<

array_view_ho.h: dirs $(INCLUDE_DIR)/$(LIB_PREFIX)/array_view.h \
		$(SRC_DIR)/array_view.c
	@echo "/* Header-only version of d4f/$(subst _ho,,$@) */" > $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@cat $(INCLUDE_DIR)/$(LIB_PREFIX)/$(subst _ho,,$@) >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@echo "\n#ifdef D4F_ARRAY_VIEW_IMPLEMENTATION" >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@cat $(SRC_DIR)/$(subst .h,.c,$(subst _ho,,$@)) | grep -v "#define D4F_ARRAY_VIEW_IMPLEMENTATION" | grep -v '#include "d4f/$(subst _ho,,$@)"' >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@echo "\n#endif" >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@

array_view.test: array_view_ho.h
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ -I$(INCLUDE_DIR) $(TEST_DIR)/$@.c

array_view: array_view.a array_view.so array_view_ho.h array_view.test
	@true

array.o: dirs array_view.o
	$(CC) $(CFLAGS) -c \
		-o $(BUILD_DIR)/array.o \
		-I$(INCLUDE_DIR) \
		$(SRC_DIR)/array.c -fPIC

array.a: array.o array_view.o
	ar -r $(LIB_DIR)/lib$(LIB_PREFIX)array.a \
		$(BUILD_DIR)/array.o $(BUILD_DIR)/array_view.o

array.so: array.o array_view.o
	$(CC) -shared -o $(LIB_DIR)/lib$(LIB_PREFIX)array.so \
		$(BUILD_DIR)/array.o $(BUILD_DIR)/array_view.o

array_ho.h: dirs $(INCLUDE_DIR)/$(LIB_PREFIX)/array_view.h \
		$(INCLUDE_DIR)/$(LIB_PREFIX)/array.h \
		$(SRC_DIR)/array.c $(SRC_DIR)/array_view.c
	@echo "/* Header-only version of d4f/$(subst _ho,_view,$@) */" > $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@echo "#ifdef D4F_ARRAY_IMPLEMENTATION\n#define D4F_ARRAY_VIEW_IMPLEMENTATION\n#endif" >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@cat $(INCLUDE_DIR)/$(LIB_PREFIX)/$(subst _ho,_view,$@) >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@echo "/* Header-only version of d4f/$(subst _ho,,$@) */" >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@cat $(INCLUDE_DIR)/$(LIB_PREFIX)/$(subst _ho,,$@) |\
		grep -v '#include "d4f/$(subst _ho,_view,$@)"' >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@echo "\n#ifdef D4F_ARRAY_IMPLEMENTATION" >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@cat $(SRC_DIR)/$(subst .h,.c,$(subst _ho,,$@)) |\
		grep -v '#include "d4f/$(subst _ho,_view,$@)"' |\
		grep -v "#define D4F_ARRAY_IMPLEMENTATION" |\
		grep -v '#include "d4f/$(subst _ho,,$@)"' >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@cat $(SRC_DIR)/$(subst .h,.c,$(subst _ho,_view,$@)) |\
		grep -v "#define D4F_ARRAY_VIEW_IMPLEMENTATION" |\
		grep -v '#include "d4f/$(subst _ho,_view,$@)"' >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@
	@echo "\n#endif" >> $(INCLUDE_DIR)/$(LIB_PREFIX)/$@

array.test: array_ho.h
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ -I$(INCLUDE_DIR) $(TEST_DIR)/$@.c

array: array.a array.so array_ho.h array.test
	@true

dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(INCLUDE_DIR)/$(LIB_PREFIX)
	mkdir -p $(LIB_DIR)

# Universal Ctags: recurse + C-only. BSD ctags (macOS default): explicit file list via xargs.
tags:
	@paths="$(SRC_DIR) $(TEST_DIR)"; \
	[ -d "$(INCLUDE_DIR)" ] && paths="$$paths $(INCLUDE_DIR)"; \
	if $(CTAGS) --version >/dev/null 2>&1; then \
	  $(CTAGS) --options=.ctags -f tags -R $$paths; \
	else \
	  if ! find $$paths \( \( -name '*.c' -o -name '*.h' \) ! -name '*_ho.h' \) 2>/dev/null | grep -q .; then \
	    echo "ctags: no .c/.h under $$paths"; exit 1; \
	  fi; \
	  find $$paths \( \( -name '*.c' -o -name '*.h' \) ! -name '*_ho.h' \) 2>/dev/null | sort -u | xargs $(CTAGS) -f tags; \
	fi

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(INCLUDE_DIR)/$(LIB_PREFIX)/*_ho.h
	rm -rf $(LIB_DIR)

