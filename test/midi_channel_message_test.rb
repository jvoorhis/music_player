require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MIDIChannelMessageHelper < Test::Unit::TestCase
  include AudioToolbox
  
  def test_initialization
    assert_raise(ArgumentError) { MIDIChannelMessage.new }
    assert_raise(ArgumentError) { MIDIChannelMessage.new({}) }
    assert_nothing_raised { MIDIChannelMessage.new :status => 42 }
  end
  
  def test_accessors
    msg = MIDIChannelMessage.new( # All arbitrary, non-default values.
            :status => status = 42,
            :data1  => data1  = 43,
            :data2  => data2  = 44)
    assert_equal status, msg.status
    assert_equal data1, msg.data1
    assert_equal data2, msg.data2
  end
end
