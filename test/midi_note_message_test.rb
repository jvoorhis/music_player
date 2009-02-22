require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MIDINoteMessageHelper < Test::Unit::TestCase
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
    msg = MIDINoteMessage.new( # All arbitrary, non-default values.
            :note             => note             = 50,
            :velocity         => velocity         = 50,
            :release_velocity => release_velocity = 30,
            :duration         => duration         = 2.0)
    assert_equal note, msg.note
    assert_equal velocity, msg.velocity
    assert_equal release_velocity, msg.release_velocity
    assert_equal duration, msg.duration
  end
end
