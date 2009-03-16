require 'music_player'

class DrumMachine
  include AudioToolbox
  
  def initialize
    @player   = MusicPlayer.new
    @sequence = MusicSequence.new
    @track    = @sequence.tracks.new
    @track.add(0.0, MIDIProgramChangeMessage.new(:channel => 10, :program => 26))
    @track.add(0.0, MIDIControlChangeMessage.new(:channel => 10, :number => 32, :value => 1))
    # Use the following call sequence to use an alternate midi destination.
    # Hopefully a more complete interface will be implemented soon. MIDI
    # destinations are referenced by their index beginning at 0.
    # See also CoreMIDI::get_number_of_destinations().
    # 
    # @sequence.midi_endpoint = CoreMIDI.get_destination(ARGV.shift.to_i)
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
    drum(beat, 35)
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
    
    @track.length = 16
    @track.loop_info = { :duration => @track.length, :number => 0 }
  end
  
  def run
    @player.start
    puts "Press return to exit."
    gets
    @player.stop
  end
end

DrumMachine.new.run
