# frozen_string_literal: true

require_relative "lib/bloomury/version"

Gem::Specification.new do |spec|
  spec.name = "bloomury"
  spec.version = Bloomury::VERSION
  spec.authors = ["Steven Scott"]
  spec.email = ["steven.bradley.scott@gmail.com"]

  spec.summary = "Ruby gem bloom filter using murmurhash3 in a C extension."
  spec.homepage = "https://github.com/tostart-pickagreatname/bloomury"
  spec.license = "MIT"
  spec.required_ruby_version = ">= 3.2.0"
  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["changelog_uri"] = "https://github.com/tostart-pickagreatname/bloomury/blob/main/CHANGELOG.md"

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  gemspec = File.basename(__FILE__)
  spec.files = IO.popen(%w[git ls-files -z], chdir: __dir__, err: IO::NULL) do |ls|
    ls.readlines("\x0", chomp: true).reject do |f|
      (f == gemspec) ||
        f.start_with?(*%w[bin/ Gemfile .gitignore test/ .rubocop.yml CLAUDE.md])
    end
  end
  spec.bindir = "exe"
  spec.executables = spec.files.grep(%r{\Aexe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]
  spec.extensions = ["ext/bloomury/extconf.rb"]
end

