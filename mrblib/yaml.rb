module YAML
  class AliasesNotEnabled < StandardError; end
  class AnchorNotDefined < StandardError; end
  class GeneratorError < StandardError; end
  class SyntaxError < StandardError; end

  class << self
    if Object.const_defined?(:IO)
      def load_file(filename, opts = {})
        YAML.load(IO.read(filename), opts)
      end
    end

    def color_object_key
      @color_object_key ||= :blue
    end

    def color_string
      @color_string ||= :green
    end

    def color_null
      @color_null ||= :gray
    end

    attr_writer :color_object_key, :color_string, :color_null
  end
end
