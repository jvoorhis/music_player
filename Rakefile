task :examples do
  Dir['examples/*.rb'].each { |example|
    `ruby -Iext #{example}`
  }
end

task :default => :examples
