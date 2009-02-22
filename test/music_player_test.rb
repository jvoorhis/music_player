require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MusicPlayerTest < Test::Unit::TestCase
  def setup
    @player = MusicPlayer.new
    @sequence = MusicSequence.new
    @track = MusicTrack.new(@sequence)
    @track.add_midi_channel_message 0.0,
        MIDIProgramChangeMessage.new(:channel => 0, :program => 1)
    @track.add_midi_note_message 0.0, MIDINoteMessage.new(:note => 60)
    @track.add_midi_note_message 1.0, MIDINoteMessage.new(:note => 64)
    @track.add_midi_note_message 2.0, MIDINoteMessage.new(:note => 67)
    @player.sequence = @sequence
  end
  
  def test_playback
    assert_nothing_raised { assert_nil @player.start }
    assert @player.playing?
    assert_nothing_raised { assert_nil @player.stop }
    assert !@player.playing?
  end

  def test_play_rate_scalar
    assert_nothing_raised do
      assert_equal 1.0, @player.play_rate_scalar
      @player.play_rate_scalar = 1.6
      assert_equal 1.6, @player.play_rate_scalar
    end
  end
end
