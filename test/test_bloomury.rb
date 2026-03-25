# frozen_string_literal: true

require "test_helper"

class TestBloomury < Minitest::Test
  test "it has a version number" do
    refute_nil ::Bloomury::VERSION
  end

  # --- argument validation ---

  test "negative capacity raises ArgumentError" do
    assert_raises(ArgumentError) { Bloomury::Filter.new(-1, 0.01) }
  end

  test "zero capacity raises ArgumentError" do
    assert_raises(ArgumentError) { Bloomury::Filter.new(0, 0.01) }
  end

  test "zero error rate raises ArgumentError" do
    assert_raises(ArgumentError) { Bloomury::Filter.new(1000, 0) }
  end

  test "error rate of one raises ArgumentError" do
    assert_raises(ArgumentError) { Bloomury::Filter.new(1000, 1) }
  end

  test "error rate above one raises ArgumentError" do
    assert_raises(ArgumentError) { Bloomury::Filter.new(1000, 1.5) }
  end

  test "negative error rate raises ArgumentError" do
    assert_raises(ArgumentError) { Bloomury::Filter.new(1000, -0.01) }
  end

  test "infeasible allocation raises RangeError" do
    assert_raises(RangeError) { Bloomury::Filter.new(1e18, 1e-300) }
  end

  # --- normal usage ---

  test "include? returns true after add" do
    f = Bloomury::Filter.new(1000, 0.01)
    f.add("hello")
    assert f.include?("hello")
  end

  test "include? returns false for missing item" do
    f = Bloomury::Filter.new(1000, 0.01)
    refute f.include?("hello")
  end

  test "count increments on add" do
    f = Bloomury::Filter.new(1000, 0.01)
    assert_equal 0, f.count
    f.add("a")
    assert_equal 1, f.count
    f.add("b")
    assert_equal 2, f.count
  end

  test "bit_count is positive" do
    f = Bloomury::Filter.new(1000, 0.01)
    assert_operator f.bit_count, :>, 0
  end

  test "hash_count is positive" do
    f = Bloomury::Filter.new(1000, 0.01)
    assert_operator f.hash_count, :>, 0
  end
end
