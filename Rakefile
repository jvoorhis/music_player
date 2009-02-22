require 'rake/testtask'

task :examples do
  Dir['examples/*.rb'].each do |example|
    system('ruby', '-Iext', example)
  end
end

task :build do
  Dir.chdir('ext') do
    system('ruby', 'extconf.rb')
    system('make')
  end
end

task :clean do
  Dir.chdir('ext') do
    system('make', 'clean')
  end
end

Rake::TestTask.new do |t|
  t.libs << 'ext'
  t.test_files = FileList['test/*_test.rb']
  t.verbose = true
end

task :test => :build # Always test the latest build.

task :default => :test
