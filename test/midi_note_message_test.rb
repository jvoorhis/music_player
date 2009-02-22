require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MIDINoteMessageTest < Test::Unit::TestCase
  include AudioToolbox
  
  def test_initialization
    assert_raise(ArgumentError) { MIDINoteMessage.new }
    assert_raise(ArgumentError) { MIDINoteMessage.new({}) }
    assert_nothing_raised { MIDINoteMessage.new :note => 60 }
    assert_nothing_raised do
      # Do your own range-checking. This is harmless.
      oob = rand(10000)
      msg = MIDINoteMessage.new :note => oob
      assert_equal (oob%256), msg.note
    end
  end
  
  def test_defaults
    msg = MIDINoteMessage.new :note => 60
    assert_equal 1, msg.channel
    assert_equal 64, msg.velocity
    assert_equal 0, msg.release_velocity
    assert_equal 1.0, msg.duration
  end

  def test_accessors
    msg = MIDINoteMessage.new(
            :note             => 50,
            :velocity         => 40,
            :release_velocity => 30,
            :duration         => 2.0)
    assert_equal 50, msg.note
    assert_equal 40, msg.velocity
    assert_equal 30, msg.release_velocity
    assert_equal 2.0, msg.duration
  end
end
