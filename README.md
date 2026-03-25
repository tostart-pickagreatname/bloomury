# Bloomury

![Bloomury poster](assets/bloomury_poster.svg)

Bloomury is a fast, memory-efficient way to answer one question: **have I seen this before?**

It's a Bloom filter — a data structure that can tell you with certainty when something is *new*, and with very high confidence when something has been *seen before*. It uses a fraction of the memory a full set would require, making it useful for deduplication, caching, and spam filtering at scale.

The trade-off: it can occasionally say "yes, I've seen this" when it hasn't (a false positive). You control how often that happens via the error rate. It will **never** say "no" when it should say "yes".

## Quick start

```ruby
gem install bloomury
```

```ruby
require "bloomury"

# Create a filter for up to 10,000 items with a 1% false positive rate
filter = Bloomury::Filter.new(10_000, 0.01)

filter.add("user@example.com")

filter.include?("user@example.com")  # => true  (definitely seen)
filter.include?("other@example.com") # => false  (definitely not seen)
```

That's it. Pick a capacity roughly equal to the number of items you expect to add, choose an error rate, and go.

## Choosing your parameters

**Capacity** is the number of items you plan to add. Going significantly over capacity increases the false positive rate beyond what you asked for.

**Seeds** control which bit positions each item maps to inside the filter. The defaults are fine for local, in-process use. If you're exposing a filter over a network where untrusted parties can observe query results, supply random seeds so an adversary can't predict the bit layout:

```ruby
require "securerandom"

filter = Bloomury::Filter.new(10_000, 0.01,
  seed1: SecureRandom.random_number(0xFFFFFFFF),
  seed2: SecureRandom.random_number(0xFFFFFFFF)
)
```

**Error rate** is the probability of a false positive once the filter is full. `0.01` means roughly 1 in 100 membership checks on unseen items will incorrectly return `true`. Lower is more accurate but uses more memory.

Not sure how much memory you'll use? Check before allocating:

```bash
rake 'memory_estimate[capacity,error_rate]'
```

```
$ rake 'memory_estimate[10000,0.01]'
capacity:   10000 items
error rate: 0.01
bit count:  95851 (7 hash functions)
memory:     11982 bytes (11.7 KB)
```

## Full API

```ruby
filter = Bloomury::Filter.new(capacity, error_rate)

filter.add("item")        # add an item
filter.include?("item")   # true/false membership check
filter.add_count          # number of add calls made (including duplicates)
filter.bit_count          # size of the underlying bit array
filter.hash_count         # number of hash functions in use
```

## Why MurmurHash3

A Bloom filter needs to hash each item multiple times, quickly, with results spread evenly across the bit array. MurmurHash3 is a good fit: it's fast, has excellent distribution, and is simple enough to embed directly in the extension with no external dependency.

It's not a cryptographic hash — it wasn't designed to be hard to reverse or to resist deliberate collisions. That's an acceptable trade-off here because speed and distribution quality matter more than security for this use case.

## When to use a Bloom filter

Good fits:
- Deduplication — skip processing URLs, emails, or IDs you've already handled
- Cache guard — avoid expensive lookups for items you know aren't cached
- Spam/abuse — fast pre-check before a heavier database query

Not a good fit:
- You need to remove items (Bloom filters are add-only)
- You need to count occurrences
- False positives are completely unacceptable

## Security considerations

### Fixed hash seeds

This implementation uses fixed seeds for MurmurHash3 (`0x9747b28c` and `0x5a4afe17`). This is fine for local, in-process use — deduplication pipelines, caches, offline processing.

If you expose a filter over a network where untrusted parties can observe query results, fixed seeds are a liability: an adversary who knows the seeds can craft inputs that saturate specific bit positions and force false positives on legitimate lookups. For network-exposed filters, random seeds are needed. Contributions welcome.

### Not a cryptographic primitive

MurmurHash3 is optimised for speed and distribution, not security. Don't use it for password hashing or message authentication.

## Development

```bash
bin/setup        # install dependencies
rake             # compile, test, lint
rake compile     # build C extension only
rake test        # Ruby tests only
rake test_c      # C unit tests only
```

## Contributing

Bug reports and pull requests are welcome at https://github.com/stscott/bloomury. Please follow the [code of conduct](https://github.com/stscott/bloomury/blob/master/CODE_OF_CONDUCT.md).

## License

[MIT License](https://opensource.org/licenses/MIT).
