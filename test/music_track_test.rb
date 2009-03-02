require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MusicTrackTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    @track = @sequence.tracks.new
  end

  def test_iterator
    assert_kind_of MusicEventIterator, @track.iterator
  end

  def test_enumerable
    @track.add 0, ev1=MIDINoteMessage.new(:note => 60)
    @track.add 1, ev2=MIDINoteMessage.new(:note => 64)
    @track.add 2, ev3=MIDINoteMessage.new(:note => 67)
    
    assert_equal [ev1, ev2, ev3], @track.map { |x| x }
  end
end
