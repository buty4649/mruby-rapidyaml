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
  assert('null') do
    assert_equal(nil, YAML.load(''), 'null string')
    %w[null Null NULL ~].each do |value|
      assert_equal(nil, YAML.load(value), value)
    end
  end

  assert('Boolean') do
    %w[true True TRUE].each do |value|
      assert_equal(true, YAML.load(value), value)
    end

    %w[false False FALSE].each do |value|
      assert_equal(false, YAML.load(value), value)
    end
  end

  assert_equal(42, YAML.load('42'), 'fixnum')
  assert_equal(12_345_678_901_234_567_890, YAML.load('12345678901234567890'), 'fixnum (big number)')

  assert_equal(3.14, YAML.load('3.14'), 'float')
  assert('NaN') do
    %w[.nan .NaN .NAN].each do |value|
      assert_true(YAML.load(value).nan?, value)
    end
  end
  assert('Infinity') do
    %w[.inf .Inf .INF +.inf +.Inf +.INF].each do |value|
      assert_equal(Float::INFINITY, YAML.load(value), value)
    end

    %w[-.inf -.Inf -.INF].each do |value|
      assert_equal(-Float::INFINITY, YAML.load(value), value)
    end
  end

  assert('String') do
    assert_equal('hello world', YAML.load("hello\nworld"), 'plain')
    assert_equal("hello\nworld", YAML.load('"hello\nworld"'), 'double quote')
    assert_equal('hello\nworld', YAML.load(%q('hello\nworld')), 'sinble quote')

    assert_equal("This is a folded string\n", YAML.load(<<-YAML), 'folded style')
    >
      This is a folded
      string
    YAML
    assert_equal('This is a folded string', YAML.load(<<-YAML), 'folded style with >-')
    >-
      This is a folded
      string
    YAML
    assert_equal("This is a folded string\n\n", YAML.load(<<-YAML), 'folded style with >+')
    >+
      This is a folded
      string

    YAML

    assert_equal("This is a literal\nstring\nwith line breaks\n", YAML.load(<<-YAML), 'literal style')
    |
      This is a literal
      string
      with line breaks
    YAML
    assert_equal("This is a literal\nstring\nwith line breaks", YAML.load(<<-YAML), 'literal style with >-')
    |-
      This is a literal
      string
      with line breaks
    YAML
    assert_equal("This is a literal\nstring\nwith line breaks\n\n", YAML.load(<<-YAML), 'literal style with >+')
    |+
      This is a literal
      string
      with line breaks

    YAML

    assert_equal("\0", YAML.load(%q("\0")), 'null character')
    assert_equal("\a", YAML.load(%q("\a")), 'bell/alert')
    assert_equal("\b", YAML.load(%q("\b")), 'backspace')
    assert_equal("\t", YAML.load(%q("\t")), 'tab character')
    assert_equal("\t", YAML.load(%q("\x09")), 'tab character (hex)')
    assert_equal("\n", YAML.load(%q("\n")), 'newline character')
    assert_equal("\v", YAML.load(%q("\v")), 'vertical tab')
    assert_equal("\f", YAML.load(%q("\f")), 'form feed')
    assert_equal("\r", YAML.load(%q("\r")), 'carriage return')
    assert_equal("\e", YAML.load(%q("\e")), 'escape character')
    assert_equal(' ', YAML.load('"\\ "'), 'space')
    assert_equal('"', YAML.load('"\\""'), 'double quote')
    assert_equal('/', YAML.load('"\\/"'), 'slash')
    assert_equal('\\', YAML.load('"\\\\"'), 'backslash')
    assert_equal("\u0085", YAML.load(%q("\N")), 'next line (NEL)')
    assert_equal("\u00A0", YAML.load(%q("\_")), 'non-breaking space')
    assert_equal("\u2028", YAML.load(%q("\L")), 'line separator')
    assert_equal("\u2029", YAML.load(%q("\P")), 'paragraph separator')

    assert_equal('A', YAML.load(%q("\x41")), 'character A with \x')
    assert_equal('あ', YAML.load(%q("\u3042")), 'character あ with \u')
    assert_equal('🍣', YAML.load(%q("\U0001F363")), 'sushi emoji with \U')
  end

  assert('Array') do
    assert_equal(%w[foo bar baz], YAML.load('["foo", "bar", "baz"]'), 'flow style')
    assert_equal(%w[foo bar baz], YAML.load(<<-YAML), 'block style')
    - foo
    - bar
    - baz
    YAML

    assert_equal([{ 'foo' => 'Foo' }, { 'bar' => 'Bar' }], YAML.load(<<-YAML), 'Array of Hashes')
    - foo: Foo
    - {"bar": "Bar"}
    YAML
  end

  assert('Hash') do
    assert_equal({ 'foo' => 'bar' }, YAML.load('{foo: bar}'), 'flow style with double quote')
    assert_equal({ "foo\nbar" => 'bar' }, YAML.load(%q({"foo\nbar": "bar"})), 'flow style with double quote')
    assert_equal({ 'foo\nbar' => 'bar' }, YAML.load(%q({'foo\nbar': 'bar'})), 'flow style with single quote')

    expect = { 'foo' => 'bar', nil => 'null', "foo\nbar" => 'foobar', 'abc\ndef' => 'abcdef' }
    assert_equal(expect, YAML.load(<<~'YAML'), 'block style')
      foo: bar
      null: "null"
      "foo\nbar": foobar
      'abc\ndef': abcdef
    YAML
    assert_equal({ "foo\nbar" => 'foobar' }, YAML.load(%q("foo\nbar": foobar)), 'block style')

    assert_equal({ 'people' => %w[Alice Bob] }, YAML.load(<<~YAML), 'Hash with Array')
      people:
        - Alice
        - Bob
    YAML
    assert_equal({ [{ 'foo' => 'bar' }] => 'baz' }, YAML.load(<<~YAML), 'Hash with Array key (block style)')
      ? - foo: bar
      : baz
    YAML
    assert_equal({ %w[foo bar] => 'foobar' }, YAML.load('[foo, bar]: foobar'), 'Hash with Array key (flow style)')
    assert_equal({ { 'foo' => 'bar' } => 'foobar' }, YAML.load('{foo: bar}: foobar'), 'Hash with Hash key')
  end

  assert_raise_with_message(YAML::SyntaxError, 'ERROR: missing terminating ]') { YAML.load('[') }
end

assert('YAML.#load_file') do
  skip unless Object.const_defined?(:IO)

  assert_equal({ 'mruby' => 'rapidyaml' }, YAML.load_file('test/fixtures/test.yaml'), 'test.yml')
end
