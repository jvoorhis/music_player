require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MIDIChannelMessageTest < Test::Unit::TestCase
  def test_initialization
    assert_raise(ArgumentError) { MIDIChannelMessage.new }
    assert_raise(ArgumentError) { MIDIChannelMessage.new({}) }
    assert_nothing_raised { MIDIChannelMessage.new :status => 42 }
  end
  
  def test_eq
    msg1 = make_msg
    msg2 = make_msg
    # Demonstrates that separate Ruby MIDIChannelMessage objects may be
    # compared by value.
    assert msg1.object_id != msg2.object_id
    assert msg1 == msg2
  end
  
  def test_accessors
    msg = make_msg
    assert_equal 42, msg.status
    assert_equal 43, msg.data1
    assert_equal 44, msg.data2
  end
  
  def make_msg
    MIDIChannelMessage.new(:status => 42, :data1  => 43, :data2  => 44)
  end
end

class MIDIKeyPressureMessageTest < Test::Unit::TestCase
  def test_initialization
    msg = MIDIKeyPressureMessage.new(
            :channel  => 1,
            :note     => 60,
            :pressure => 64)
    
    assert_equal 1, msg.channel
    assert_equal 60, msg.note
    assert_equal 64, msg.pressure
    assert_equal msg.mask | msg.channel, msg.status
  end
end

class MIDIControlChangeMessageTest < Test::Unit::TestCase
  def test_initialization
    msg = MIDIControlChangeMessage.new(
            :channel => 1,
            :number  => 8,
            :value   => 64)
    
    assert_equal 1, msg.channel
    assert_equal 8, msg.number
    assert_equal 64, msg.value
    assert_equal msg.mask | msg.channel, msg.status
  end
end

class MIDIProgramChangeMessageTest < Test::Unit::TestCase
  def test_initialization
    msg = MIDIProgramChangeMessage.new(
            :channel => 1,
            :program => 2)
    
    assert_equal 1, msg.channel
    assert_equal 2, msg.program
    assert_equal msg.mask | msg.channel, msg.status
  end
end

class MIDIChannelPressureMessageTest < Test::Unit::TestCase
  def test_initialization
    msg = MIDIChannelPressureMessage.new(
            :channel  => 1,
            :pressure => 64)
    
    assert_equal 1, msg.channel
    assert_equal 64, msg.pressure
    assert_equal msg.mask | msg.channel, msg.status
  end
end

class MIDIPitchBendMessageTest < Test::Unit::TestCase
  def test_initialization
    msg = MIDIPitchBendMessage.new(
            :channel => 1,
            :value   => 64)
    
    assert_equal 1, msg.channel
    assert_equal 64, msg.value
    assert_equal msg.mask | msg.channel, msg.status
  end
end
