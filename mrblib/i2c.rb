#
# PlatoNRF52::I2C class
#
module PlatoNRF52
  class I2C
    include Plato::I2C
  
    # i2c.read(cmd, datlen, type) => Array/String
    def read(cmd, datlen, type=:as_array)
      v = nil
      if write(cmd)
        Plato::Machine.delay(@wait) if @wait > 0
        v = _read(datlen, type)
      end
      v
    end
  end
end

# register nRF52 device
Plato::I2C.register_device(PlatoNRF52::I2C)
