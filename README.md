# StreamSeal — Minimal, Auditable File Encryption (Argon2id → ChaCha20-Poly1305 / Secretstream) — v2

[![CI](https://img.shields.io/github/actions/workflow/status/RaiMUmar/StreamSeal/ci.yml?label=CI&logo=github)](https://github.com/RaiMUmar/StreamSeal/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
![C99](https://img.shields.io/badge/C-99-00599C?logo=c&logoColor=white)

> A cross-platform CLI that encrypts/decrypts files and folders using **Argon2id** (KDF) and **ChaCha20-Poly1305**.  
> v2 adds **streamed encryption** via libsodium **secretstream** (constant-memory), **opt-in deletion**, **atomic 0600 writes** for credentials, **symlink/device skipping**, and **corruption tests** — all with a small, auditable C codebase.

---

## ✨ Why this exists

- **Practical**: one binary, simple commands: `init-user`, `encrypt`, `decrypt`.
- **Secure defaults**: Argon2id + authenticated encryption; header bound as AAD in v2.
- **Readable**: small C codebase, unit tests, corruption tests, fuzz smoke, sanitizers, CI.

---

## ⚡ Quick start

```bash
# 1) Build
make              # or: SAN=asan make test   (address/UB sanitizers)

# 2) Initialize credentials (writes ./user.pass with 0600, atomically)
./bin/vault init-user

# 3) Encrypt a file (streaming). Non-destructive by default.
./bin/vault encrypt path/to/plain.txt            # writes path/to/plain.enc
./bin/vault encrypt path/to/dir                  # recursive; skips symlinks/devices
./bin/vault encrypt path/to/plain.txt --rm       # also deletes plaintext on success

# 4) Decrypt (streaming). Non-destructive by default.
./bin/vault decrypt path/to/plain.enc            # writes path/to/plain.dec
./bin/vault decrypt path/to/plain.enc .out --rm  # custom suffix; delete .enc on success
```

**Notes**
- **Deletion is now opt-in** (`--rm` or `--delete`). Without it, sources are preserved.
- **Symlinks and special files** (devices/FIFOs/sockets) are **skipped**; directories recurse.
- `user.pass` is written **atomically** with final permissions **0600**; perms are tightened on login if needed.

---

## 🧠 Threat model (what it protects / doesn’t)

**In scope**

- Confidentiality & integrity of file contents against an attacker.
- Strong password hashing with **Argon2id** (libsodium moderate limits).
- **v2 streamed format:** header fields (magic, version, KDF params, salt) are **bound as AAD**; tampering causes decryption failure.

**Out of scope / limitations**

- **Filenames, sizes, and directory structure are visible** (metadata leakage).
- **No secure deletion**: `remove()` unlinks only; data may be recoverable on some filesystems.
- Active malware/compromised host when you run the tool; keylogging/side-channels.
- Key rotation, file sharing, multi-user policies.

---

## 🧱 File formats

### v2 — **Streaming format** (default)

```
+--------------------+-------------------------------+
| stream_hdr_t       |  Secretstream ciphertext ...  |
|  (bound as AAD)    |  (chunked, final tag)         |
+--------------------+-------------------------------+
```

**`stream_hdr_t` overview**

| Field               | Size                         | Purpose                                   |
|---------------------|------------------------------|-------------------------------------------|
| `magic`             | 6 bytes                      | format ID (v2)                            |
| `version`           | 4 bytes (u32)                | format version                            |
| `kdf_mem_kib`       | 4 bytes (u32)                | Argon2id mem limit (KiB)                  |
| `kdf_opslimit`      | 4 bytes (u32)                | Argon2id ops limit                        |
| `salt`              | 16 bytes                     | KDF salt                                  |
| `ss_header`         | 24 bytes (libsodium constant)| secretstream header                       |

> The **AAD** is the header **up to** (but not including) `ss_header`, so magic/version/KDF params/salt are cryptographically bound.

**Why streaming?**
- Constant memory usage for large files.
- Early tamper detection; decryption fails if any chunk is corrupted.

### v1 — Legacy simple format (still decryptable)

```
+--------------------+---------------------------+
| simple_hdr_t       |  AEAD ciphertext blob     |
+--------------------+---------------------------+
```

- Still supported on **decrypt** for backward compatibility.
- v2 is preferred for new data.

---

## 🔑 KDF & crypto parameters

- **KDF**: libsodium `crypto_pwhash` (Argon2id, `ALG_ARGON2ID13`)
  - Ops/memory: `OPSLIMIT_MODERATE`, `MEMLIMIT_MODERATE`
  - v2 records **KDF params** in the header; decrypt uses those exact values.
- **AEAD/stream**: `crypto_secretstream_xchacha20poly1305`
  - Per-file random salt; per-stream `ss_header`
  - Final chunk carries a **FINAL** tag
- **Zeroization**: `sodium_memzero` on password and derived keys

---

## 🛠️ CLI behavior & flags

**Commands**
- `init-user` — create `user.pass` with Argon2id hash (atomic, 0600)
- `encrypt <path> [--rm|--delete]` — file or directory (recursive); writes `<name>.enc`
- `decrypt <path> [suffix] [--rm|--delete]` — writes `<base><suffix>` (default `.dec`)

**Behavior**
- **Opt-in delete**: add `--rm` to remove sources on success.
- **Symlinks/devices**: **skipped**. Directories recurse. `user.pass` is never processed.
- Decrypt auto-detects format (v2 streaming vs v1 simple) by header magic.

---

## 🧪 Tests & CI

- **Unit tests**: path building, round-trip (encrypt/decrypt)
- **Corruption tests**: header and payload tamper → decryption fails; quiet logs
- **Fuzz smoke**: random inputs into decryptor (no crashes)
- **Static analysis**: `cppcheck`, `codespell`
- **Sanitizers**: Address/UB
- **CI**: GitHub Actions matrix (macOS + Ubuntu)

Run locally:
```bash
make test                    # builds and runs all tests
SAN=asan make test           # with sanitizers
```

---

## 🚧 Roadmap

- **Archive mode**: pack folders into a single stream to **hide filenames/structure** (e.g., tar-style stream + encryption).
- Passphrase sources: `--pass-file`, `--pass-env`, `--pass-fd`
- Non-interactive mode, better exit codes, verbose logging
- Secure-delete adapters (best-effort, clearly documented caveats)
- Key rotation tooling

---

## 📦 Building

```bash
# Dependencies: libsodium, clang/gcc, make, pkg-config
# macOS:  brew install libsodium pkg-config
# Ubuntu: sudo apt-get install -y clang pkg-config libsodium-dev

make
make test
```

---

## 🔐 Security notes

- Use a **strong, unique passphrase**. Only the Argon2id hash is stored (`user.pass`).
- `user.pass` is written **atomically** and should remain **0600**.
- There is **no secure deletion**. If you must wipe contents, research and use OS/FS-appropriate tools (with caveats).

---

## 📄 License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE).

---

## 📊 Changelog

- **v2.0**
  - **Streaming encryption/decryption** (secretstream, constant memory, header AAD-bound)
  - **Opt-in deletion** via `--rm`/`--delete`
  - **Atomic 0600** write for `user.pass`; permission tightening on login
  - **Skip symlinks/devices**; don’t follow symlinks
  - **Corruption tests** added; tests output “All Tests Passed!” on success
- **v1.0**
  - Initial public release: AEAD with ChaCha20-Poly1305 (IETF), Argon2id KDF, recursive encrypt/decrypt, tests, and CI.
