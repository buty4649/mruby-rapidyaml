assert('Object#to_yaml') do
  assert_equal('null', nil.to_yaml, 'nil')
  assert_equal('true', true.to_yaml, 'true')
  assert_equal('false', false.to_yaml, 'false')
  assert_equal('42', 42.to_yaml, 'fixnum')
  assert_equal('3.14', 3.14.to_yaml, 'float')
  assert_equal('.nan', Float::NAN.to_yaml, 'nan')
  assert_equal('-.inf', (-Float::INFINITY).to_yaml, '-inf')
  assert_equal('.inf', Float::INFINITY.to_yaml, 'inf')
  assert_equal('hello', 'hello'.to_yaml, 'String')
  assert_equal("- 1\n- 2\n- 3", [1, 2, 3].to_yaml, 'Array')
  assert_equal("name: Alice\nage: 30", { 'name' => 'Alice', 'age' => 30 }.to_yaml, 'Hash')
end
