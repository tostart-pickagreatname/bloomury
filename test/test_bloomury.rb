# frozen_string_literal: true

require "test_helper"

class TestBloomury < Minitest::Test
  def test_that_it_has_a_version_number
    refute_nil ::Bloomury::VERSION
  end

  # --- argument validation ---

  def test_negative_capacity_raises
    assert_raises(ArgumentError) { Bloomury::Filter.new(-1, 0.01) }
  end

  def test_zero_capacity_raises
    assert_raises(ArgumentError) { Bloomury::Filter.new(0, 0.01) }
  end

  def test_zero_error_rate_raises
    assert_raises(ArgumentError) { Bloomury::Filter.new(1000, 0) }
  end

  def test_error_rate_of_one_raises
    assert_raises(ArgumentError) { Bloomury::Filter.new(1000, 1) }
  end

  def test_error_rate_above_one_raises
    assert_raises(ArgumentError) { Bloomury::Filter.new(1000, 1.5) }
  end

  def test_negative_error_rate_raises
    assert_raises(ArgumentError) { Bloomury::Filter.new(1000, -0.01) }
  end

  def test_infeasible_allocation_raises
    assert_raises(RangeError) { Bloomury::Filter.new(1e18, 1e-300) }
  end

  # --- normal usage ---

  def test_include_returns_true_after_add
    f = Bloomury::Filter.new(1000, 0.01)
    f.add("hello")
    assert f.include?("hello")
  end

  def test_include_returns_false_for_missing_item
    f = Bloomury::Filter.new(1000, 0.01)
    refute f.include?("hello")
  end

  def test_count_increments_on_add
    f = Bloomury::Filter.new(1000, 0.01)
    assert_equal 0, f.count
    f.add("a")
    assert_equal 1, f.count
    f.add("b")
    assert_equal 2, f.count
  end

  def test_bit_count_is_positive
    f = Bloomury::Filter.new(1000, 0.01)
    assert_operator f.bit_count, :>, 0
  end

  def test_hash_count_is_positive
    f = Bloomury::Filter.new(1000, 0.01)
    assert_operator f.hash_count, :>, 0
  end
end
