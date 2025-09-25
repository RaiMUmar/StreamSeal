# Vault — Minimal, Auditable File Encryption (Argon2id → ChaCha20‑Poly1305)

[![CI](https://img.shields.io/github/actions/workflow/status/RaiMUmar/StreamSeal/ci.yml?label=CI&logo=github)](https://github.com/RaiMUmar/StreamSeal/actions)
[![CodeQL](https://img.shields.io/github/actions/workflow/status/RaiMUmar/StreamSeal/codeql.yml?label=CodeQL&logo=github)](https://github.com/RaiMUmar/StreamSeal/actions?query=workflow%3ACodeQL)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
![C99](https://img.shields.io/badge/C-99-00599C?logo=c&logoColor=white)

> A cross‑platform CLI that encrypts/decrypts files and folders using **Argon2id key‑derivation** and **ChaCha20‑Poly1305 (IETF)** AEAD.
> Built for clarity and correctness first: small codebase, unit tests, fuzz smoke, and GitHub Actions CI (macOS + Ubuntu).

---

## ✨ Why this exists

- **Practical**: one binary, simple commands: `init-user`, `encrypt`, `decrypt`.
- **Secure defaults** (crypto): Argon2id → ChaCha20‑Poly1305, authenticated encryption.
- **Readable**: short C code with tests, static analysis, sanitizers, and a documented threat model.

> **Note:** Current implementation uses whole‑file I/O (reads the full file into memory). Streaming/chunked I/O is on the roadmap.

---

## ⚡ Quick start

```bash
# 1) Build
make            # or: SAN=asan make test   (address/UB sanitizers)

# 2) Initialize credentials (writes ./user.pass)
./bin/vault init-user

# 3) Encrypt a file (creates <name>.enc, removes plaintext on success)
./bin/vault encrypt path/to/plain.txt

# 4) Decrypt (creates <name>.dec by default, removes .enc on success)
./bin/vault decrypt path/to/plain.enc .dec
```

**Important:** By default this tool deletes the source file on success (plaintext after encrypt; ciphertext after decrypt). This is intentional for minimal “vault‑style” flows but can be surprising. If you prefer a non‑destructive flow, duplicate your data or adapt the code path (see _Design & Flags_).

---

## 🧠 Threat model (what it protects / doesn’t)

**In scope**

- Confidentiality & integrity of file contents against an attacker who obtains the encrypted files but **not** your password.
- Strong password hashing with **Argon2id** (moderate limits) and authenticated encryption with **ChaCha20‑Poly1305 (IETF)**.
- Tamper detection of ciphertext (decryption fails if modified).

**Out of scope / limitations**

- **Filenames, sizes, and directory structure are visible** (metadata leakage).
- **No secure deletion**: `remove()` unlinks only; content may remain recoverable on disk/FS.
- **Whole‑file I/O**: large files require enough RAM to hold the entire content.
- **Active malware or compromised host** at encryption/decryption time.
- Side channels (timing, memory access patterns) and keylogging are not addressed.
- Key rotation and multi‑user sharing are not implemented.

---

## 🧱 File format & header diagram

Each encrypted file is:

```
+--------------------+---------------------------+
| simple_hdr_t (34B) |  ChaCha20-Poly1305 blob  |
+--------------------+---------------------------+
```

### `simple_hdr_t` (packed for clarity)

| Field          | Bytes | Description                                     |
|----------------|------:|-------------------------------------------------|
| `magic`        |     6 | ASCII `"SIMPL1"` to identify the format         |
| `salt`         |    16 | Random salt for Argon2id                        |
| `nonce`        |    12 | Unique nonce for ChaCha20‑Poly1305 (IETF)       |

> Integrity is provided by AEAD; header is validated by `magic`, but (in this version) **not cryptographically bound as AAD**. See Roadmap for AAD‑bound, versioned headers.

---

## 🔑 KDF & crypto parameters (current)

- **KDF**: `crypto_pwhash` with **Argon2id** (`crypto_pwhash_ALG_ARGON2ID13`)
  - Ops/memory: `OPSLIMIT_MODERATE`, `MEMLIMIT_MODERATE` (libsodium defaults)
- **AEAD**: `crypto_aead_chacha20poly1305_ietf_*`
  - Nonce: 12 bytes from `randombytes_buf` (per file)
  - Auth tag: 16 bytes appended by libsodium
- **Zeroization**: Sensitive buffers (`key`, `pwd`) are wiped with `sodium_memzero`

> Tune KDF limits for your hardware if needed. Future versions may record KDF params in the header.

---

## 🧪 Tests & CI

- **Unit tests** for path building and encrypt/decrypt round‑trip
- **Fuzz smoke** target that feeds random bytes into the decryptor (should not crash)
- **Sanitizers**: Address + Undefined behavior
- **Static analysis**: `cppcheck`, `codespell`
- **GitHub Actions**: macOS + Ubuntu matrix

CI badges above assume:
- CI workflow file (e.g., `.github/workflows/ci.yml`) — update `<WORKFLOW_FILE>` as needed
- Optional CodeQL scanning file at `.github/workflows/codeql.yml`

---

## 🛠️ Design & flags (CLI)

**Commands**

- `init-user` — creates `user.pass` containing Argon2id hash of your password
- `encrypt <path>` — encrypt file or folder (recursive) → writes `<name>.enc` by default
- `decrypt <path> [suffix]` — decrypt file → writes `<base><suffix>` (default `.dec`)

**Behavior**

- On **success**, source file is removed (plaintext after encrypt, ciphertext after decrypt).
- Special files: currently regular files are processed; folders are recursed. `user.pass` and certain suffixes may be skipped by policy.

**Planned flags (roadmap)**

- `--rm` (opt‑in deletion), `--dry-run`, `--pass-file`, `--pass-env`, `--verbose`

---

## 🚧 Limitations & future work

- **Streaming/chunked I/O** via `crypto_secretstream_xchacha20poly1305` (constant memory)
- **AAD‑bound, versioned headers** (bind header fields; store KDF params)
- **Non‑destructive default** (make deletion opt‑in)
- **Symlink/device handling** hardening
- Optional **archive mode** to hide filenames/structure

---

## 📦 Building

```bash
# Dependencies: libsodium, clang (or gcc), make, pkg-config
# macOS:  brew install libsodium pkg-config
# Ubuntu: sudo apt-get install -y clang pkg-config libsodium-dev

make            # build all
make test       # unit tests + fuzz smoke
SAN=asan make   # with ASan/UBSan
```

---

## 🔐 Security notes

- Choose a **strong, unique password**. Password is never stored; only Argon2id hash.
- `user.pass` permissions should be **0600**; atomic write is recommended in code.
- Secure deletion is **not** provided by default; use external tools if needed and understand FS caveats.

---

## 📄 License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE).

---

## 🙌 Acknowledgements

- [libsodium](https://github.com/jedisct1/libsodium) — modern, easy‑to‑use crypto
- AEAD design inspired by best practices in libsodium docs

---

## 📊 Changelog (excerpt)

- **v1.0** — Initial public release: AEAD with ChaCha20‑Poly1305 (IETF), Argon2id KDF, recursive encrypt/decrypt, tests, and CI.
