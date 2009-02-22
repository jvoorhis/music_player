require 'audio_toolbox'

class DrumMachine
  include AudioToolbox
  
  def initialize
    @player   = MusicPlayer.new
    @sequence = MusicSequence.new
    @track    = MusicTrack.new(@sequence)
    @track.add_midi_channel_message(0.0,
      MIDIProgramChangeMessage.new(:channel => 10, :program => 26))
    @track.add_midi_channel_message(0.0,
      MIDIControlChangeMessage.new(:channel => 10, :number => 32, :value => 1))

    @player.sequence = @sequence
    build_track
  end
  
  def play(beat, note, velocity=80, channel=10)
    @track.add_midi_note_message(beat,
      MIDINoteMessage.new(:note     => note,
                          :velocity => velocity,
                          :channel  => channel,
                          :duration => 0.1))
  end
  
  KICK1 = 32
  KICK2 = 36
  SNARE = 40
  
  def build_track
    (0..15).each do |beat|
      play(beat, KICK1) # every downbeat
      play(beat+0.5, KICK2) # every upbeat
      if beat % 4 == 0
        extra = (beat % 8)/4
        play(beat+extra, SNARE)
      end
    end
  end
  
  def run
    @player.start
    puts "Press return to exit."
    gets
    @player.stop
  end
end

DrumMachine.new.run
