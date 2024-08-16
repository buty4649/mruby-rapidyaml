module YAML
  class AliasesNotEnabled < StandardError; end
  class AnchorNotDefined < StandardError; end
  class GeneratorError < StandardError; end
  class SyntaxError < StandardError; end

  if Object.const_defined?(:IO)
    def self.load_file(filename, opts = {})
      YAML.load(IO.read(filename), opts)
    end
  end

  @color_map_key = %i[blue cyan magenta red]
  class << self
    attr_writer :color_boolean, :color_string, :color_null
    attr_accessor :color_number

    def color_map_key(level)
      raise ArgumentError, "level must be greater than or equal to 1, but got #{level}" unless level >= 1

      @color_map_key[(level - 1) % @color_map_key.size]
    end

    def set_color_map_key(level, color)
      raise ArgumentError, "level must be between 1 and 4, but got #{level}" unless level >= 1 && level <= 4

      @color_map_key[level - 1] = color
    end

    def color_boolean
      @color_boolean ||= :yellow
    end

    def color_null
      @color_null ||= :gray
    end

    def color_string
      @color_string ||= :green
    end
  end
end
