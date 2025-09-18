PKGCONF ?= pkg-config
CFLAGS_COMMON = -std=c99 -Wall -pedantic $(shell $(PKGCONF) --cflags libsodium)
LDFLAGS = $(shell $(PKGCONF) --libs libsodium)

vault: main.o vault_decrypt.o vault_encrypt.o vault_io.o vault_login.o vault_print_hex.o vault_usage.o vault_path_handler.o vault_decrypt_inplace.o vault_encrypt_inplace.o vault_delete.o vault_build_path.o vault_util.o
	clang ./vault_util.o ./vault_build_path.o ./vault_delete.o ./vault_encrypt_inplace.o ./vault_decrypt_inplace.o ./vault_path_handler.o ./main.o ./vault_decrypt.o ./vault_encrypt.o ./vault_io.o ./vault_login.o ./vault_print_hex.o ./vault_usage.o $(LDFLAGS) -o bin/vault

vault_path_handler.o: ./src/vault_path_handler.c 
	clang $(CFLAGS_COMMON) -I./include ./src/vault_path_handler.c -c

main.o: ./src/main.c
	clang $(CFLAGS_COMMON) -I./include ./src/main.c -c

vault_decrypt.o: ./src/vault_decrypt.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_decrypt.c -c

vault_encrypt.o: ./src/vault_encrypt.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_encrypt.c -c

vault_io.o: ./src/vault_io.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_io.c -c

vault_login.o: ./src/vault_login.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_login.c -c

vault_print_hex.o: ./src/vault_print_hex.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_print_hex.c -c

vault_usage.o: ./src/vault_usage.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_usage.c -c

vault_build_path.o: ./src/vault_build_path.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_build_path.c -c

vault_delete.o: ./src/vault_delete.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_delete.c -c

vault_encrypt_inplace.o: ./src/vault_encrypt_inplace.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_encrypt_inplace.c -c

vault_decrypt_inplace.o: ./src/vault_decrypt_inplace.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_decrypt_inplace.c -c

vault_util.o: ./src/vault_util.c
	clang $(CFLAGS_COMMON) -I./include ./src/vault_util.c -c

clean:
	rm -f *.o *.pass bin/vault