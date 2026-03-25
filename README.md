# Bloomury

A probabilistic set membership filter backed by a native C extension. Uses double-hashing with MurmurHash3 (32-bit) and two fixed seeds to generate k independent hash positions per item, striking a balance between speed and low false-positive rates.

The filter is parameterised by capacity and error rate; optimal bit count (m) and hash count (k) are derived at initialisation using the standard formulae. The bit array is heap-allocated and wrapped via Ruby's TypedData API for safe memory management.

## Usage

```ruby
filter = Bloomury::Filter.new(capacity, error_rate)
filter.add("hello")
filter.include?("hello")  # => true
filter.include?("world")  # => false (probably)
filter.count              # number of items added
filter.bit_count          # size of the bit array
filter.hash_count         # number of hash functions (k)
```

## Installation

```bash
bundle add bloomury
```

Or without bundler:

```bash
gem install bloomury
```

## Security considerations

### Hash seeds

This implementation uses fixed seeds for MurmurHash3 (`0x9747b28c` and `0x5a4afe17`). This is appropriate for the intended use cases — local in-process set membership testing, privacy pipelines, deduplication — where no adversary has the ability to probe the filter.

If you expose a bloom filter over a network interface where untrusted parties can observe membership query results, fixed seeds make you vulnerable to a HashDoS attack. An adversary who knows the seeds can craft inputs that hash to the same bit positions, deliberately saturating regions of the filter to produce false positives on legitimate lookups.

For network-exposed filters, initialize with random seeds and store them alongside the serialized filter state. Contributions welcome.

### Not a cryptographic primitive

MurmurHash3 is a non-cryptographic hash function optimized for speed and distribution quality. It is not suitable for password hashing, message authentication, or any use case requiring preimage resistance. 

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake test` to run the tests. You can also run `bin/console` for an interactive prompt.

```bash
rake          # clobber, compile, test, rubocop
rake compile  # build C extension
rake test     # run tests only
```

To estimate how much memory a filter will consume before allocating one:

```bash
rake 'memory_estimate[capacity,error_rate]'
```

```
$ rake 'memory_estimate[1000000,0.01]'
capacity:   1000000 items
error rate: 0.01
bit count:  9585059 (7 hash functions)
memory:     1198133 bytes (1170.05 KB)
```

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/stscott/bloomury. Contributors are expected to adhere to the [code of conduct](https://github.com/stscott/bloomury/blob/master/CODE_OF_CONDUCT.md).

## License

Available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
