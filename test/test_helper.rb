# frozen_string_literal: true

$LOAD_PATH.unshift File.expand_path("../lib", __dir__)
require "bloomury"

require "minitest/autorun"

class Minitest::Test
  def self.test(name, &block)
    define_method("test_#{name.gsub(/[^a-z0-9]+/i, "_")}", &block)
  end
end
