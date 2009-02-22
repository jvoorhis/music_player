require File.join(File.dirname(__FILE__), 'test_helper.rb')

class ExtendedTempoEventTest < Test::Unit::TestCase
  def test_initialization
    event = ExtendedTempoEvent.new(:bpm => 120)
    assert_equal 120, event.bpm
  end
  
  def test_add
    sequence = MusicSequence.new
    tempo = sequence.tracks.tempo
    event = ExtendedTempoEvent.new(:bpm => 120)
    assert_nothing_raised { event.add(0.0, tempo) }
  end
end
