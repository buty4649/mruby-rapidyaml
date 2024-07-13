class Object
  def to_yaml(opts = {})
    YAML.dump(self, opts)
  end
end
