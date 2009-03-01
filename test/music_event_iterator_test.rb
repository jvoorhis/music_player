require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MusicEventIteratorTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    @track = @sequence.tracks.new
    @track.add 0.0, @ev1=MIDINoteMessage.new(:note => 60)
    @track.add 1.0, @ev2=MIDINoteMessage.new(:note => 67)
    @iter = @track.iterator
  end
  
  def test_seek
    assert_nothing_raised { @iter.seek(0.0) }
  end
  
  def test_current?
    assert @iter.current?
    @iter.next
    assert @iter.current?
    @iter.next
    assert !@iter.current?
  end
  
  def test_next
    assert_nothing_raised do
      @iter.next; @iter.next
    end
    assert_raise(EndOfTrack) { @iter.next }
  end
  
  def test_next?
    assert @iter.next?
    @iter.next
    assert !@iter.next?
    @iter.next # We may advance once beyond the last event.
    assert_raise(EndOfTrack) { @iter.next }
  end
  
  def test_prev
    assert_nothing_raised do
      @iter.next; @iter.prev
    end
    assert_raise(StartOfTrack) { @iter.prev }
  end
  
  def test_prev?
    assert !@iter.prev?
    @iter.next
    assert @iter.prev?
  end

  def test_time
    assert_equal 0.0, @iter.time
    @iter.next
    assert_equal 1.0, @iter.time
    @iter.next
    assert_raise(EndOfTrack) { @iter.time }
    @iter.seek(0.0)
    assert_equal 0.0, @iter.time
  end

  def test_event__note
    assert_nothing_raised do
      assert_equal @ev1, @iter.event
      @iter.next
      assert_equal @ev2, @iter.event
    end
  end
  
  def test_event__channel
    track = @sequence.tracks.new
    track.add 0, ev1=MIDIKeyPressureMessage.new(:channel => 1, :note => 60, :pressure => 64)
    track.add 0, ev2=MIDIControlChangeMessage.new(:channel => 1, :number => 1, :value => 127)
    track.add 0, ev3=MIDIProgramChangeMessage.new(:channel => 1, :program => 42)
    track.add 0, ev4=MIDIChannelPressureMessage.new(:channel => 1, :pressure => 37)
    track.add 0, ev5=MIDIPitchBendMessage.new(:channel => 1, :value => 84)
    iter = track.iterator
    
    assert_equal ev1, iter.event
    iter.next
    assert_equal ev2, iter.event
    iter.next
    assert_equal ev3, iter.event
    iter.next
    assert_equal ev4, iter.event
    iter.next
    assert_equal ev5, iter.event
  end
  
  def test_event__tempo
    track = @sequence.tracks.tempo
    track.add 0, ev=ExtendedTempoEvent.new(:bpm => 120)
    iter = track.iterator
    assert_equal ev, iter.event
  end
end
