assert('Object#to_yaml') do
  obj = Object.new
  opts = {}

  stub = lambda do |v, o|
    assert_equal obj, v
    assert_equal opts, o
    'stub'
  end

  YAML.stub(:dump, stub) do
    assert_equal 'stub', obj.to_yaml(opts)
  end
end
