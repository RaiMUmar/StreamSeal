CC ?= clang
PKGCONF ?= pkg-config

UNAME_S := $(shell uname -s)

# Dirs
SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj
BIN_DIR := bin

RUNS ?= 1000

# Common flags (no POSIX macro here)
CFLAGS_COMMON = -std=c99 -Wall -pedantic -g \
                $(shell $(PKGCONF) --cflags libsodium)
LDFLAGS      = $(shell $(PKGCONF) --libs libsodium)

# Add POSIX feature macro only on Linux
ifeq ($(UNAME_S),Linux)
  CFLAGS_COMMON += -D_POSIX_C_SOURCE=200809L
endif

# Source files for the main binary
SRC_FILES := \
  main.c \
  vault_decrypt.c \
  vault_encrypt.c \
  vault_io.c \
  vault_login.c \
  vault_print_hex.c \
  vault_usage.c \
  vault_path_handler.c \
  vault_decrypt_inplace.c \
  vault_encrypt_inplace.c \
  vault_delete.c \
  vault_build_path.c \
  vault_util.c \
  vault_prompt_password.c \
  vault_stream.c \
  vault_globals.c

SRCS := $(addprefix $(SRC_DIR)/,$(SRC_FILES))
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# Default target
all: $(BIN_DIR)/vault

# Build the main binary from obj files
$(BIN_DIR)/vault: | $(BIN_DIR) $(OBJ_DIR) $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Pattern rule: any .c in src -> .o in obj (with dep files)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS_COMMON) -MMD -MP -c $< -o $@

# Ensure directories exist
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ---- Tests ----
TESTS := $(BIN_DIR)/test_build_path $(BIN_DIR)/test_roundtrip $(BIN_DIR)/test_corruption

$(BIN_DIR)/test_build_path: tests/test_build_path.c $(SRC_DIR)/vault_build_path.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_COMMON) $^ $(LDFLAGS) -o $@

$(BIN_DIR)/test_corruption: tests/test_corruption.c \
                           src/vault_stream.c src/vault_io.c src/vault_util.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_COMMON) -I./include $^ $(LDFLAGS) -o $@

# Include vault_stream.c because encrypt/decrypt_inplace use streaming now
$(BIN_DIR)/test_roundtrip: tests/test_roundtrip.c \
                           $(SRC_DIR)/vault_encrypt_inplace.c $(SRC_DIR)/vault_decrypt_inplace.c \
                           $(SRC_DIR)/vault_encrypt.c $(SRC_DIR)/vault_decrypt.c $(SRC_DIR)/vault_io.c \
                           $(SRC_DIR)/vault_build_path.c $(SRC_DIR)/vault_delete.c $(SRC_DIR)/vault_util.c \
                           $(SRC_DIR)/vault_stream.c $(SRC_DIR)/vault_globals.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_COMMON) $^ $(LDFLAGS) -o $@

ifeq ($(SAN),asan)
  CFLAGS_COMMON += -fsanitize=address,undefined -fno-omit-frame-pointer
  LDFLAGS      += -fsanitize=address,undefined
  TEST_ENV      = MallocNanoZone=0
endif

.PHONY: test
test: $(TESTS)
	@set -e; for t in $(TESTS); do echo ">>> $$t"; $(TEST_ENV) "$$t"; done; printf "\033[1;32mAll Tests Passed!\033[0m\n"

.PHONY: cppcheck codespell
cppcheck:
	cppcheck --enable=warning,performance,portability --std=c11 --quiet $(SRC_DIR) $(INC_DIR)

codespell:
	codespell -S .git,$(BIN_DIR),$(OBJ_DIR) -q 3 -L clen

# ---- Fuzz smoke ----
FUZZ_CFLAGS  = -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer
FUZZ_LDFLAGS = -fsanitize=address,undefined

$(BIN_DIR)/fuzz_smoke: tests/fuzz_smoke.c $(SRC_DIR)/vault_decrypt.c $(SRC_DIR)/vault_io.c $(SRC_DIR)/vault_util.c
	@mkdir -p $(BIN_DIR)
	$(CC) -std=c99 $(FUZZ_CFLAGS) -I./$(INC_DIR) \
	      $(shell $(PKGCONF) --cflags libsodium) \
	      $^ \
	      $(shell $(PKGCONF) --libs libsodium) $(FUZZ_LDFLAGS) \
	      -o $@

.PHONY: fuzz-smoke
fuzz-smoke: $(BIN_DIR)/fuzz_smoke
	@echo ">>> fuzz smoke ($(RUNS) runs)"
	@MallocNanoZone=0 $(BIN_DIR)/fuzz_smoke -runs=$(RUNS)

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d *.pass
	rm -rf $(BIN_DIR) $(OBJ_DIR)

# Include auto-generated dependency files
-include $(DEPS)

