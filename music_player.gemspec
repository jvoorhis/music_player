Gem::Specification.new do |s|
  s.name = "music_player"
  s.version = "0.9.0"
  s.author = "Jeremy Voorhis"
  s.email = "jvoorhis@gmail.com"
  s.homepage = "http://github.com/jvoorhis/music_player"
  s.summary = s.description =
    "Ruby bindings for OS X's high-quality music sequencing API"
  # s.rubyforge_project = "music_player"
  
  s.files = Dir.glob %w[
    LICENSE
    Rakefile
    examples/**/*.rb
    ext/music_player/**/*.{c,h,rb}
    lib/**/*.rb
    musicplayer.gemspec
    test/**/*.{mid,rb}
  ]
  s.extensions = ['ext/music_player/extconf.rb']
  s.platform = Gem::Platform::CURRENT
  s.require_paths = %w[ lib ext ]
  s.test_files = Dir.glob 'test/*_test.rb'
end
