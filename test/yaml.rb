assert('YAML.#dump') do
  assert_equal('null', YAML.dump(nil), 'nil')
  assert_equal('true', YAML.dump(true), 'true')
  assert_equal('false', YAML.dump(false), 'false')
  assert_equal('42', YAML.dump(42), 'fixnum')
  assert_equal('3.14', YAML.dump(3.14), 'float')
  assert_equal('.nan', YAML.dump(Float::NAN), 'nan')
  assert_equal('-.inf', YAML.dump(-Float::INFINITY), '-inf')
  assert_equal('.inf', YAML.dump(Float::INFINITY), 'inf')
  assert_equal('hello', YAML.dump('hello'), 'String')
  assert_equal("- 1\n- 2\n- 3", YAML.dump([1, 2, 3]), 'Array')
  assert_equal("name: Alice\nage: 30", YAML.dump({ 'name' => 'Alice', 'age' => 30 }), 'Hash')
end

assert('YAML.#load') do
  assert_equal(nil, YAML.load('null'), 'nil')
  assert_equal(true, YAML.load('true'), 'true')
  assert_equal(false, YAML.load('false'), 'false')
  assert_equal(42, YAML.load('42'), 'fixnum')
  assert_equal(3.14, YAML.load('3.14'), 'float')
  assert_true(YAML.load('.nan').nan?, 'nan')
  assert_equal(-Float::INFINITY, YAML.load('-.inf'), '-inf')
  assert_equal(Float::INFINITY, YAML.load('.inf'), 'inf')
  assert_equal('hello', YAML.load('hello'), 'String')
  assert_equal([1, 2, 3], YAML.load("- 1\n- 2\n- 3"), 'Array')
  assert_equal({ 'name' => 'Alice', 'age' => 30 }, YAML.load("name: Alice\nage: 30"), 'Hash')
  assert_equal({ [1, 2] => 3 }, YAML.load('[1, 2]: 3'), 'Hash with Array key')
  assert_equal({ { '1' => 2 } => 3 }, YAML.load('{1: 2}: 3'), 'Hash with Hash key')

  assert_raise_with_message(YAML::SyntaxError, 'ERROR: missing terminating ]') { YAML.load('[') }
end

assert('YAML.#load_file') do
  skip unless Object.const_defined?(:IO)

  assert_equal({ 'mruby' => 'rapidyaml' }, YAML.load_file('test/fixtures/test.yaml'), 'test.yml')
end
