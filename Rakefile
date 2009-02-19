task :examples do
  Dir['examples/*.rb'].each do |example|
    system('ruby', '-Iext', example)
  end
end

task :build do
  Dir.chdir('ext') { system('make') }
end

task :default => :examples
