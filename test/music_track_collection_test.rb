class MusicSequenceTracksEnumerableTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    @track1 = @sequence.tracks.new
    @track2 = @sequence.tracks.new
    @track3 = @sequence.tracks.new
  end
  
  def test_size
    assert_equal 3, @sequence.tracks.size
  end
  
  def test_at_index
    assert @sequence.tracks[0]
    assert_raise(RangeError) { @sequence.tracks[42] }
    
    assert_equal @sequence.tracks[0].object_id,
                 @sequence.tracks[0].object_id
  end
  
  def test_each
    trks = []
    @sequence.tracks.each do |trk|
      trks << trk
    end
    # Separate references can be modified independently.
    # trks.first.add(0.0, MIDINoteMessage.new(:note => 60))
    # @track1.add(1.0, MIDINoteMessage.new(:note => 72))
    assert_equal [@track1, @track2, @track3], trks
  end
  
  def test_index
    assert_equal 0, @sequence.tracks.index(@track1)
    assert_raise(ArgumentError) { @sequence.tracks.index(0) }
    
    seq = MusicSequence.new
    track = seq.tracks.new
    assert_raise(RangeError) { @sequence.tracks.index(track) }
  end
  
  def test_delete
    assert_equal 3, @sequence.tracks.size
    trk = @sequence.tracks[0]
    @sequence.tracks.delete(trk)
    assert_equal 2, @sequence.tracks.size
  end
  
  def test_tempo
    assert_kind_of MusicTrack, @sequence.tracks.tempo
    assert_equal @sequence.tracks.tempo, @sequence.tracks.tempo,
      "Expected only one instance of the tempo track."
  end
end
