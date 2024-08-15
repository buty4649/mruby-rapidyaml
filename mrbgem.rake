MRuby::Gem::Specification.new('mruby-rapidyaml') do |spec|
  spec.license = 'MIT'
  spec.authors = 'buty4649@gmail.com'
  spec.summary = 'rapidyaml bindings for mruby'
  spec.description = 'rapidyaml bindings for mruby'
  spec.version = '1.0.0'

  spec.cxx.flags << '-std=c++11'
  spec.cxx.defines << %w[C4_WIN] if Gem.win_platform?

  spec.add_dependency 'mruby-terminal-color', github: 'buty4649/mruby-terminal-color', branch: 'main'

  spec.add_test_dependency 'mruby-io', core: 'mruby-io'
  spec.add_test_dependency 'mruby-test-stub', github: 'buty4649/mruby-test-stub', branch: 'main'
end
