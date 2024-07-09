# mruby-rapidyaml

[rapidyaml](https://github.com/biojppm/rapidyaml/) binding for mruby

## Build Requirements

To build mruby-rapidyaml, you will need the following:

* C++11 compatible compiler
  - clang++ 3.9 and later
  - g++ 4.8 and later

Note: rapidyaml is written in C++11, hence a C++11 compatible compiler is required.

## Installation

Add the following line to your `build_config.rb`:

```ruby
MRuby::Build.new do |conf|
-- snip --

  conf.gem github: 'buty4649/mruby-rapidyaml'
end
```

## Implemented Methods

| Method     | mruby-rapidyaml | Description                    |
|------------|-----------------|--------------------------------|
| YAML.#dump | ✓               |                                |
| YAML.#load | ✓               |                                |

## License

mruby-rapidyaml is licensed under the MIT License.
mruby-rapidyaml includes code from rapidyaml([ryml_all.hpp](src/ryml_all.hpp)), which is also licensed under the MIT License.
For more details, see the [LICENSE](./LICENSE) file.
