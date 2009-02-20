require 'rake/testtask'

task :examples do
  Dir['examples/*.rb'].each do |example|
    system('ruby', '-Iext', example)
  end
end

task :build do
  Dir.chdir('ext') { system('make') }
end

Rake::TestTask.new do |t|
  t.libs << 'ext'
  t.test_files = FileList['test/*_test.rb']
  t.verbose = true
end

task :default => :test
