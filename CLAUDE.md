# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

```bash
bin/setup               # Install dependencies
rake                    # Default: clobber, compile, test, test_c, rubocop
rake compile            # Build C extension
rake test               # Ruby tests only (Minitest)
rake test_c             # C unit tests only (test/c/test_murmurhash3.c)
rake rubocop            # Lint Ruby code
rake 'memory_estimate[capacity,error_rate]'  # Estimate memory usage before allocating
```

To run a single Ruby test:
```bash
ruby -Ilib:test test/test_bloomury.rb --name "test name here"
```

## Architecture

This is a Ruby gem with a C native extension implementing a Bloom filter.

**C layer** (`ext/bloomury/`):
- `murmurhash3.c/.h` — standalone MurmurHash3 (32-bit) implementation; no external dependencies
- `bloom_filter.c` — `BloomFilter` struct logic: bit array storage, `bloom_filter_add`, `bloom_filter_check`. Uses double-hashing (`h1 + i*h2`) with two seeds to generate `k` bit positions per item.
- `ruby_api.c` — Ruby C API bindings; defines `Bloomury::Filter` class via `TypedData_Wrap_Struct`. All `rb_define_method` calls are here.
- `bloomury.h` — shared header; defines the `BloomFilter` struct (`bits`, `bit_count`, `hash_count`, `item_count`, `seed1`, `seed2`)
- `extconf.rb` — uses `mkmf`, sets `-fvisibility=hidden` and `-O2`

**Ruby layer** (`lib/`):
- `Bloomury::Filter` is entirely implemented in C; the Ruby layer only loads the `.so` and defines the `Bloomury::Error` class.

**Public API** (`Bloomury::Filter`):
- `new(capacity, error_rate, seed1:, seed2:)` — capacity/error_rate drive the m/k parameter calculation; seeds default to `0x9747b28c` / `0x5a4afe17`
- `add(item)`, `include?(item)`, `add_count`, `bit_count`, `hash_count`, `seed1`, `seed2`

**Tests**:
- `test/test_bloomury.rb` — Minitest Ruby tests
- `test/c/test_murmurhash3.c` — standalone C tests compiled and run by `rake test_c`
