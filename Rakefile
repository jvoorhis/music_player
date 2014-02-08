require 'fileutils'
require 'rake/testtask'
require 'rubygems/package_task'
require 'rbconfig'

def rb_cmd
  @rb_cmd ||= File.join(RbConfig::CONFIG['bindir'],
                        RbConfig::CONFIG['ruby_install_name']
                        ).sub(/.*\s.*/m, '"\&"')
end

task :build do
  Dir.chdir('ext') do
    system(rb_cmd, 'music_player/extconf.rb')
    system('make')
  end
end

task :clean do
  Dir['ext/*{Makefile,.o,.bundle}'].each do |path|
    FileUtils.rm_rf(path)
  end
end

Rake::TestTask.new do |t|
  t.libs << 'ext'
  t.test_files = FileList['test/*_test.rb']
  t.verbose = true
end

task :test => :build # Always test the latest build.

spec = eval open('music_player.gemspec').read
Gem::PackageTask.new spec do |pkg| end

task :default => :test
