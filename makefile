vault: main.o vault_decrypt.o vault_encrypt.o vault_io.o vault_login.o vault_print_hex.o vault_usage.o
	clang ./main.o ./vault_decrypt.o ./vault_encrypt.o ./vault_io.o ./vault_login.o ./vault_print_hex.o ./vault_usage.o -o bin/vault

main.o: ./src/main.c
	clang -std=c99 -Wall -pedantic  -I./include ./src/main.c -c

vault_decrypt.o: ./src/vault_decrypt.c
	clang -std=c99 -Wall -pedantic -I./include ./src/vault_decrypt.c -c

vault_encrypt.o: ./src/vault_encrypt.c
	clang -std=c99 -Wall -pedantic -I./include ./src/vault_encrypt.c -c

vault_io.o: ./src/vault_io.c
	clang -std=c99 -Wall -pedantic -I./include ./src/vault_io.c -c

vault_login.o: ./src/vault_login.c
	clang -std=c99 -Wall -pedantic -I./include ./src/vault_login.c -c

vault_print_hex.o: ./src/vault_print_hex.c
	clang -std=c99 -Wall -pedantic -I./include ./src/vault_print_hex.c -c

vault_usage.o: ./src/vault_usage.c
	clang -std=c99 -Wall -pedantic -I./include ./src/vault_usage.c -c