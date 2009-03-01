$:.unshift File.join(File.dirname(__FILE__), '../ext')
require 'thread'
require 'music_player.bundle'

module AudioToolbox
  class MusicSequence
    attr :tracks
    
    def load(path)
      @tracks.lock.synchronize do
        load_internal(path)
      end
    end
  end
  
  class MusicTrackCollection
    include Enumerable
    
    attr :lock # :nodoc:
    
    def initialize(sequence)
      @sequence = sequence
      @tracks   = []
      @lock     = Mutex.new
    end
    
    def each
      0.upto(size-1) { |i| yield self[i] }
    end
    
    def new
      @lock.synchronize do
        track = MusicTrack.send(:new, @sequence)
        @tracks << track
        track
      end
    end
    
    def [](index)
      @lock.synchronize do
        @tracks[index] ||= ind_internal(index)
      end
    end
    
    def delete(track)
      @lock.synchronize do
        delete_internal(track)
        @tracks.delete(track)
        track.freeze
      end
      nil
    end
    
    def tempo
      @tempo ||= tempo_internal
    end
  end
  
  class MusicTrack
    class << self
      private :new
    end
    
    def add(time, message)
      message.add(time, self)
    end

    def iterator
      MusicEventIterator.new(self)
    end
  end
  
  class MIDINoteMessage
    def ==(msg)
      self.class       == msg.class &&
      channel          == msg.channel &&
      note             == msg.note &&
      velocity         == msg.velocity &&
      release_velocity == msg.release_velocity &&
      duration         == msg.duration
    end
    
    def add(time, track)
      track.add_midi_note_message(time, self)
    end
  end
  
  class MIDIChannelMessage
    def ==(msg)
      self.class == msg.class &&
      status     == msg.status &&
      data1      == msg.data1 &&
      data2      == msg.data2
    end
    
    def channel
      status ^ mask
    end
    
    def mask
      raise NotImplementedError, "Subclass responsibility."
    end
    
    def add(time, track)
      track.add_midi_channel_message(time, self)
    end
    
    protected
      def required_opts(opts, *keys)
        keys.map do |key|
          opts[key] or raise ArgumentError, "%s is required." % key.inspect
        end
      end
  end
  
  class MIDIKeyPressureMessage < MIDIChannelMessage
    alias :note :data1
    alias :pressure :data2
    
    def mask; 0xA0 end
    
    def initialize(opts)
      channel, note, pressure = required_opts(opts, :channel, :note, :pressure)
      super(:status => mask | channel,
            :data1  => note,
            :data2  => pressure)
    end
  end
  
  class MIDIControlChangeMessage < MIDIChannelMessage
    alias :number :data1
    alias :value :data2
    
    def mask; 0xB0 end
    
    def initialize(opts)
      channel, number, value = required_opts(opts, :channel, :number, :value)
      super(:status => mask | channel,
            :data1  => number,
            :data2  => value)
    end
  end
  
  class MIDIProgramChangeMessage < MIDIChannelMessage
    alias :program :data1
    
    def mask; 0xC0 end
    
    def initialize(opts)
      channel, program = required_opts(opts, :channel, :program)
      super(:status => mask | channel,
            :data1  => program)
    end
  end
  
  class MIDIChannelPressureMessage < MIDIChannelMessage
    alias :pressure :data1
    
    def mask; 0xD0 end
    
    def initialize(opts)
      channel, pressure = required_opts(opts, :channel, :pressure)
      super(:status => mask | channel,
            :data1  => pressure)
    end
  end
  
  class MIDIPitchBendMessage < MIDIChannelMessage
    alias :value :data1
    
    def mask; 0xE0 end
    
    def initialize(opts)
      channel, value = required_opts(opts, :channel, :value)
      super(:status => mask | channel,
            :data1  => value)
    end
  end
  
  class ExtendedTempoEvent
    attr :bpm
    
    def add(time, track)
      track.add_extended_tempo_event(time, @bpm)
    end
    
    def initialize(opts)
      @bpm = opts[:bpm]
    end

    def ==(other)
      self.class == other.class &&
      bpm        == other.bpm
    end
  end
end
