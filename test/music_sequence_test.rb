require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MusicSequenceTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    @track = MusicTrack.new(@sequence)
    @track.add_midi_channel_message 0.0,
        MIDIProgramChangeMessage.new(:channel => 0, :program => 1)
    @track.add_midi_note_message 0.0, MIDINoteMessage.new(:note => 60)
    @track.add_midi_note_message 1.0, MIDINoteMessage.new(:note => 64)
    @track.add_midi_note_message 2.0, MIDINoteMessage.new(:note => 67)
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
end
