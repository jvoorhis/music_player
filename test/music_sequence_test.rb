require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MusicSequenceTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    
    @tempo = @sequence.tracks.tempo
    @tempo.add 0.0, ExtendedTempoEvent.new(:bpm => 120)
    
    @track = MusicTrack.new(@sequence)
    @track.add 0.0, MIDIProgramChangeMessage.new(:channel => 0, :program => 1)
    @track.add 0.0, MIDINoteMessage.new(:note => 60)
    @track.add 1.0, MIDINoteMessage.new(:note => 64)
    @track.add 2.0, MIDINoteMessage.new(:note => 67)
  end
  
  def test_size
    assert_equal 1, @sequence.tracks.size
  end
  
  def test_at_index
    assert @sequence.tracks[0]
    assert_raise(RangeError) { @sequence.tracks[42] }
  end
  
  def test_index
    assert_equal 0, @sequence.tracks.index(@track)
    assert_raise(ArgumentError) { @sequence.tracks.index(0) }
    
    seq = MusicSequence.new
    track = MusicTrack.new(seq)
    assert_raise(RangeError) { @sequence.tracks.index(track) }
  end
  
  def test_tempo
    assert_kind_of MusicTrack, @sequence.tracks.tempo
  end
  
  def test_delete
    assert_equal 1, @sequence.tracks.size
    trk = @sequence.tracks[0]
    @sequence.tracks.delete(trk)
    assert_equal 0, @sequence.tracks.size
  end
end

class MusicSequenceTracksEnumerableTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    @track1 = MusicTrack.new(@sequence)
    @track2 = MusicTrack.new(@sequence)
    @track3 = MusicTrack.new(@sequence)
  end
  
  def test_each
    trks = []
    @sequence.tracks.each do |trk|
      trks << trk
    end
    # Separate references can be modified...
    trks.first.add(0.0, MIDINoteMessage.new(:note => 60))
    # ...independently.
    @track1.add(1.0, MIDINoteMessage.new(:note => 72))
    assert_equal [@track1, @track2, @track3], trks
  end
end
