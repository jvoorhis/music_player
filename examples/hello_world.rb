require 'music_player'
include AudioToolbox

player = MusicPlayer.new
sequence = MusicSequence.new
player.sequence = sequence
sequence.midi_endpoint = CoreMIDI.get_destination(ARGV.shift.to_i)

tempo = sequence.tracks.tempo
tempo.add 0.0, ExtendedTempoEvent.new(:bpm => 120.0)

track = sequence.tracks.new
track.add 0.0, MIDINoteMessage.new(:note => 60, :velocity => 64)
track.add 1.0, MIDINoteMessage.new(:note => 64, :velocity => 96)
track.add 2.0, MIDINoteMessage.new(:note => 67, :velocity => 110)
track.add 3.0, MIDINoteMessage.new(:note => 60, :velocity => 110, :duration => 2.0)
track.add 3.0, MIDINoteMessage.new(:note => 64, :velocity => 110, :duration => 2.0)
track.add 3.0, MIDINoteMessage.new(:note => 67, :velocity => 110, :duration => 2.0)
track.add 3.0, MIDINoteMessage.new(:note => 72, :velocity => 110, :duration => 2.0)

sequence.save("~/mytest.mid")

player.start
sleep 3
player.stop
