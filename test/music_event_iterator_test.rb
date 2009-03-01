require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MusicEventIteratorTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    @track = @sequence.tracks.new
    @track.add 0, @ev1=MIDINoteMessage.new(:note => 60)
    @track.add 1, @ev2=MIDINoteMessage.new(:note => 67)
    @iter = @track.iterator
  end
  
  def test_seek
    assert_nothing_raised { @iter.seek(0) }
    assert_equal @ev1, @iter.event
    # Seeking to a point between event onsets causes
    # the iterator to advance to the next onset.
    @iter.seek(0.1) 
    assert_equal @ev2, @iter.event
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
  
  def test_set_time
    # swap note onsets
    assert_equal 0, @iter.time
    @iter.time = 1
    assert_equal 1, @iter.time
    @iter.next
    assert_equal 1, @iter.time
    @iter.time = 0
    
    @iter.seek 0
    assert_equal @ev2, @iter.event
    @iter.next
    assert_equal @ev1, @iter.event
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
  
  def test_set_event
    # swapping notes 
    @iter.seek(0)
    @iter.event = @ev2
    @iter.next
    @iter.event = @ev1
    
    @iter.seek(0)
    assert_equal @ev2, @iter.event
    @iter.next
    assert_equal @ev1, @iter.event
    
    # Event types may be altered.
    @iter.seek(0)
    @iter.event = cc=MIDIControlChangeMessage.new(:channel => 1, :number => 2, :value => 3)
    assert_equal cc, @iter.event
    
    # Assigning when there is no current event raises an exception.
    # Use MusicTrack#add to add new events.
    @iter.next until !@iter.current?
    assert_raise(EndOfTrack) do
      @iter.event = MIDINoteMessage.new(:note => 60)
    end
    
    # Tempo tracks are iterable as well.
    tempo = @sequence.tracks.tempo
    tempo.add 0, ExtendedTempoEvent.new(:bpm => 120)
    
    iter = @sequence.tracks.tempo.iterator
    assert_equal 120, iter.event.bpm
    iter.event = ExtendedTempoEvent.new(:bpm => 60)
    assert_equal 60, iter.event.bpm
  end
  
  def test_delete
    assert_equal @ev1, @iter.event
    @iter.delete
    assert_equal @ev2, @iter.event
    @iter.delete
    
    assert !@iter.current?
    assert_nothing_raised { @iter.delete }
  end
end
