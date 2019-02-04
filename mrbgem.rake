MRuby::Gem::Specification.new('mruby-plato-i2c-nrf52') do |spec|
  spec.license = 'MIT'
  spec.author = 'Hiroshi Mimaki'
  spec.description = 'PlatoNRF52::I2C class'

  spec.add_dependency('mruby-plato-i2c')
  spec.add_test_dependency('mruby-string-ext')
end
