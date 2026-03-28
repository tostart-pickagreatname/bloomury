# frozen_string_literal: true

require "bundler/gem_tasks"

begin
  require "minitest/test_task"
  Minitest::TestTask.create
rescue LoadError
end

begin
  require "rubocop/rake_task"
  RuboCop::RakeTask.new
rescue LoadError
end

require "rake/extensiontask"

task build: :compile

GEMSPEC = Gem::Specification.load("bloomury.gemspec")

Rake::ExtensionTask.new("bloomury", GEMSPEC) do |ext|
  ext.lib_dir = "lib/bloomury"
end

desc "Estimate filter memory usage: rake memory_estimate[capacity,error_rate]"
task :memory_estimate, [:capacity, :error_rate] do |_t, args|
  capacity   = Float(args[:capacity]   || abort("capacity required"))
  error_rate = Float(args[:error_rate] || abort("error_rate required"))

  abort("error_rate must be between 0 and 1") unless (0...1).include?(error_rate)
  abort("capacity must be positive")          unless capacity.positive?

  m = (-capacity * Math.log(error_rate) / Math.log(2)**2).ceil
  k = ((m / capacity) * Math.log(2)).round

  bytes = (m / 8.0).ceil
  puts "capacity:   #{capacity.to_i} items"
  puts "error rate: #{error_rate}"
  puts "bit count:  #{m} (#{k} hash functions)"
  puts "memory:     #{bytes} bytes (#{(bytes / 1024.0).round(2)} KB)"
end

desc "Run C unit tests"
task test_c: :compile do
  cc      = RbConfig::CONFIG["CC"]
  cflags  = "-I#{File.expand_path("ext/bloomury")}"
  src     = "test/c/test_murmurhash3.c ext/bloomury/murmurhash3.c"
  out     = "tmp/test_murmurhash3"

  mkdir_p "tmp"
  sh "#{cc} #{cflags} #{src} -o #{out}"
  sh out
end

task default: %i[clobber compile test test_c rubocop]
