require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MusicTrackTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    @track = @sequence.tracks.new
  end
  
  def test_add
    assert_nothing_raised do
      @track.add 0, MIDINoteMessage.new(:note => 60)
      # a representative channel message
      @track.add 0, MIDIControlChangeMessage.new(:channel => 0, :number => 0, :value => 0)
    end
    
    assert_raise(IllegalTrackDestination) do
      @track.add 0, ExtendedTempoEvent.new(:bpm => 120)
    end
  end
  
  def test_iterator
    assert_kind_of MusicEventIterator, @track.iterator
  end
  
  def test_enumerable
    @track.add 0, ev1=MIDINoteMessage.new(:note => 60)
    @track.add 1, ev2=MIDINoteMessage.new(:note => 64)
    @track.add 2, ev3=MIDINoteMessage.new(:note => 67)
    
    assert_equal [ev1, ev2, ev3], @track.map { |x| x }
  end
  
  def test_loop_info
    assert_equal({ :duration => 0.0, :number => 1 }, @track.loop_info)
    @track.loop_info = { :duration => 100.0, :number => 42 }
    assert_equal({ :duration => 100.0, :number => 42 }, @track.loop_info)
  end
  
  def test_offset
    assert_equal 0.0, @track.offset
    @track.offset = 1.0
    assert_equal 1.0, @track.offset
  end
  
  def test_mute
    assert_equal false, @track.mute
    @track.mute = true
    assert_equal true, @track.mute
  end
  
  def test_solo
    assert_equal false, @track.solo
    @track.solo = true
    assert_equal true, @track.solo
  end
  
  def test_length
    assert_equal 0, @track.length
    
    @track.add 0, MIDINoteMessage.new(:note => 60, :duration => 1)
    assert_equal 1, @track.length
    
    # Length is maximum of the user-set length and the timestamp of the
    # release of the final event.
    @track.length = 0.5
    assert_equal 1, @track.length
    
    @track.length = 10
    assert_equal 10, @track.length
    
    @track.add 42, MIDINoteMessage.new(:note => 60, :duration => 1)
    assert_equal 43, @track.length
  end
  
  def test_resolution
    # 480 is the default resolution. 960 is also common.
    assert_equal 480, @sequence.tracks.tempo.resolution
    assert_raise(ArgumentError) { @track.resolution }
  end
  
  def test_initialize
    track = @sequence.tracks.new(:loop_info => { :duration => 10,
                                                 :number   => 3 },
                                 :mute      => true,
                                 :solo      => true,
                                 :length    => 10)
    assert_equal({:duration => 10, :number => 3}, track.loop_info)
    assert track.mute
    assert track.solo
    assert_equal 10, track.length
  end
end
