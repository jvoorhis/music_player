require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MIDINoteMessageHelper < Test::Unit::TestCase
  include AudioToolbox
  
  def test_initialization
    assert_raise(ArgumentError) { MIDINoteMessage.new }
    assert_raise(ArgumentError) { MIDINoteMessage.new({}) }
    assert_nothing_raised { MIDINoteMessage.new :note => 60 }
    assert_nothing_raised {
      # Do your own range-checking. This is harmless.
      oob = rand(10000)
      msg = MIDINoteMessage.new :note => oob
      assert_equal (oob%256), msg.note
    }
  end
  
  def test_defaults
    msg = MIDINoteMessage.new :note => 60
    assert_equal 1, msg.channel
    assert_equal 64, msg.velocity
    assert_equal 0, msg.release_velocity
    assert_equal 1.0, msg.duration
  end
end
