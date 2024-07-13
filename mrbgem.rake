MRuby::Gem::Specification.new('mruby-rapidyaml') do |spec|
  spec.license = 'MIT'
  spec.authors = 'buty4649@gmail.com'
  spec.summary = 'rapidyaml bindings for mruby'
  spec.description = 'rapidyaml bindings for mruby'
  spec.version = '1.0.0'

  spec.cxx.flags << '-std=c++11'

  spec.add_test_dependency 'mruby-io', core: 'mruby-io'
end
