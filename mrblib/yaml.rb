module YAML
  class AliasesNotEnabled < StandardError; end
  class AnchorNotDefined < StandardError; end
  class SyntaxError < StandardError; end

  if Object.const_defined?(:IO)
    def self.load_file(filename, opts = {})
      YAML.load(IO.read(filename), opts)
    end
  end
end
