require File.join(File.dirname(__FILE__), 'test_helper.rb')

class MusicTrackTest < Test::Unit::TestCase
  def setup
    @sequence = MusicSequence.new
    @track = @sequence.tracks.new
  end

  def test_iterator
    assert_kind_of MusicEventIterator, @track.iterator
  end
end
