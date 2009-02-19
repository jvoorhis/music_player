require 'music_player'
include AudioToolbox

p = MusicPlayer.new
s = MusicSequence.new
t = MusicTrack.new(s)
s.midi_endpoint = CoreMIDI.get_destination(ARGV.shift.to_i)

t.add_midi_note_message 0.0, MIDINoteMessage.new(:note => 60, :velocity => 64)
t.add_midi_note_message 1.0, MIDINoteMessage.new(:note => 64, :velocity => 96)
t.add_midi_note_message 2.0, MIDINoteMessage.new(:note => 67, :velocity => 110)

t.add_midi_note_message 3.0, MIDINoteMessage.new(:note => 60, :velocity => 110, :duration => 2.0)
t.add_midi_note_message 3.0, MIDINoteMessage.new(:note => 64, :velocity => 110, :duration => 2.0)
t.add_midi_note_message 3.0, MIDINoteMessage.new(:note => 67, :velocity => 110, :duration => 2.0)
t.add_midi_note_message 3.0, MIDINoteMessage.new(:note => 72, :velocity => 110, :duration => 2.0)

p.sequence = s
p.start
sleep 3
p.stop
