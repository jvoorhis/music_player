require 'music_player'

class DrumMachine
  include AudioToolbox
  
  def initialize
    @player   = MusicPlayer.new
    @sequence = MusicSequence.new
    @track    = @sequence.tracks.new
    @track.add(0.0, MIDIProgramChangeMessage.new(:channel => 10, :program => 26))
    @track.add(0.0, MIDIControlChangeMessage.new(:channel => 10, :number => 32, :value => 1))
    @player.sequence = @sequence
    build_track
  end
  
  def drum(beat, note)
    @track.add(beat,
      MIDINoteMessage.new(:note     => note,
                          :velocity => 80,
                          :channel  => 10,
                          :duration => 0.1))
  end
  
  def kick1(beat)
    drum(beat, 32)
  end

  def kick2(beat)
    drum(beat, 36)
  end
  
  def snare(beat)
    drum(beat, 40)
  end
  
  def build_track
    0.upto(15) do |beat|
      kick1(beat)     # every downbeat
      kick2(beat+0.5) # every upbeat
      if beat % 4 == 0
        extra = (beat % 8)/4
        snare(beat+extra)
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
