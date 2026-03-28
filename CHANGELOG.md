## [Unreleased]

## [0.1.1] - 2026-03-27

### Changed
- Scoped `minitest` to the `:test` group and `rubocop`/`irb` to the `:development` group in the Gemfile
- Rakefile now gracefully handles missing test/development gems via `rescue LoadError`
- Removed unused RBS type stub (`sig/bloomury.rbs`)
- Removed unused `bin/console` boilerplate
- Removed boilerplate comment from `lib/bloomury.rb`
- Removed `set -vx` from `bin/setup`
- Added automated gem release workflow via GitHub Actions with trusted publishing

## [0.1.0] - 2026-03-25

### Added
- `Bloomury::Filter` C extension implementing a Bloom filter with MurmurHash3 (32-bit)
- Double-hashing (`h1 + i*h2`) with configurable `k` bit positions per item
- `new(capacity, error_rate, seed1:, seed2:)` — auto-calculates optimal `m`/`k` parameters; seeds default to `0x9747b28c` / `0x5a4afe17`
- `add(item)`, `include?(item)`, `add_count`, `bit_count`, `hash_count`, `seed1`, `seed2`
- Standalone C unit tests for MurmurHash3 (`rake test_c`)
- `rake 'memory_estimate[capacity,error_rate]'` task to estimate memory usage before allocating
