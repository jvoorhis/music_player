require File.join(File.dirname(__FILE__), 'test_helper.rb')
require 'music_player'

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
  
  def test_player
    assert_nothing_raised { assert_nil @player.start }
    assert @player.playing?
    assert_nothing_raised { assert_nil @player.stop }
    assert !@player.playing?
  end
end
