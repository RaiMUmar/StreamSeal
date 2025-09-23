CC ?= clang
PKGCONF ?= pkg-config
CFLAGS_COMMON = -std=c99 -Wall -pedantic -g $(shell $(PKGCONF) --cflags libsodium)
LDFLAGS = $(shell $(PKGCONF) --libs libsodium)

BIN_DIR := bin

vault: main.o vault_decrypt.o vault_encrypt.o vault_io.o vault_login.o vault_print_hex.o \
       vault_usage.o vault_path_handler.o vault_decrypt_inplace.o vault_encrypt_inplace.o \
       vault_delete.o vault_build_path.o vault_util.o vault_prompt_password.o
	@mkdir -p $(BIN_DIR)
	$(CC) ./vault_prompt_password.o ./vault_util.o ./vault_build_path.o \
	      ./vault_delete.o ./vault_encrypt_inplace.o ./vault_decrypt_inplace.o \
	      ./vault_path_handler.o ./main.o ./vault_decrypt.o ./vault_encrypt.o \
	      ./vault_io.o ./vault_login.o ./vault_print_hex.o ./vault_usage.o \
	      $(LDFLAGS) -o $(BIN_DIR)/vault

vault_path_handler.o: ./src/vault_path_handler.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_path_handler.c -c

main.o: ./src/main.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/main.c -c

vault_decrypt.o: ./src/vault_decrypt.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_decrypt.c -c

vault_encrypt.o: ./src/vault_encrypt.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_encrypt.c -c

vault_io.o: ./src/vault_io.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_io.c -c

vault_login.o: ./src/vault_login.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_login.c -c

vault_print_hex.o: ./src/vault_print_hex.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_print_hex.c -c

vault_usage.o: ./src/vault_usage.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_usage.c -c

vault_build_path.o: ./src/vault_build_path.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_build_path.c -c

vault_delete.o: ./src/vault_delete.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_delete.c -c

vault_encrypt_inplace.o: ./src/vault_encrypt_inplace.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_encrypt_inplace.c -c

vault_decrypt_inplace.o: ./src/vault_decrypt_inplace.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_decrypt_inplace.c -c

vault_util.o: ./src/vault_util.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_util.c -c

vault_prompt_password.o: ./src/vault_prompt_password.c
	$(CC) $(CFLAGS_COMMON) -I./include ./src/vault_prompt_password.c -c


TESTS := $(BIN_DIR)/test_build_path $(BIN_DIR)/test_roundtrip

$(BIN_DIR)/test_build_path: tests/test_build_path.c src/vault_build_path.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_COMMON) -I./include $^ $(LDFLAGS) -o $@

$(BIN_DIR)/test_roundtrip: tests/test_roundtrip.c \
                           src/vault_encrypt_inplace.c src/vault_decrypt_inplace.c \
                           src/vault_encrypt.c src/vault_decrypt.c src/vault_io.c \
                           src/vault_build_path.c src/vault_delete.c src/vault_util.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_COMMON) -I./include $^ $(LDFLAGS) -o $@


ifeq ($(SAN),asan)
  CFLAGS_COMMON += -fsanitize=address,undefined -fno-omit-frame-pointer
  LDFLAGS      += -fsanitize=address,undefined
  TEST_ENV      = MallocNanoZone=0
endif

.PHONY: test
test: $(TESTS)
	@set -e; for t in $(TESTS); do echo ">>> $$t"; $(TEST_ENV) "$$t"; done; echo "OK"

.PHONY: cppcheck codespell
cppcheck:
	cppcheck --enable=warning,performance,portability --std=c11 --quiet src include

codespell:
	codespell -S .git,bin,obj -q 3 -L clen


.PHONY: clean
clean:
	rm -f *.o *.pass $(BIN_DIR)/vault
	rm -rf $(BIN_DIR)/test_* $(BIN_DIR)/*.dSYM
