require File.join(File.dirname(__FILE__), 'test_helper.rb')
require 'tempfile'

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
  
  def test_type
    assert_equal :beat, @sequence.type, "Expected default type of :beat."
    seq = MusicSequence.new
    seq.type = :samp
    assert_equal :samp, seq.type
    seq.type = :secs
    assert_equal :secs, seq.type
  end
  
  def test_save_with_pathname
    tmp = Tempfile.new('music_sequence_test.mid')
    assert_nothing_raised { @sequence.save(tmp.path) }
    assert File.exists?(tmp.path)
  end
  
  def test_tracks_size
    assert_equal 1, @sequence.tracks.size
  end
  
  def test_tracks_at_index
    assert @sequence.tracks[0]
    assert_raise(RangeError) { @sequence.tracks[42] }
  end
  
  def test_tracks_index
    assert_equal 0, @sequence.tracks.index(@track)
    assert_raise(ArgumentError) { @sequence.tracks.index(0) }
    
    seq = MusicSequence.new
    track = MusicTrack.new(seq)
    assert_raise(RangeError) { @sequence.tracks.index(track) }
  end
  
  def test_tracks_tempo
    assert_kind_of MusicTrack, @sequence.tracks.tempo
  end
  
  def test_tracks_delete
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
    # Separate references can be modified independently.
    trks.first.add(0.0, MIDINoteMessage.new(:note => 60))
    @track1.add(1.0, MIDINoteMessage.new(:note => 72))
    assert_equal [@track1, @track2, @track3], trks
  end
end
