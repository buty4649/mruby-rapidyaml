MRuby::Build.new do |conf|
  conf.toolchain

  conf.cxx.defines << %w[C4_WIN] if Gem.win_platform?

  conf.gembox 'default'
  conf.gem File.expand_path(__dir__)

  conf.enable_debug
  conf.enable_test
end
