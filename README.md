# logicantsy

Library and CLI assistant for research in logic.

## Build

```
mkdir build
cd build
cmake ../sources .
cmake --build .
```

## Test

```
make test
```

# CLI

We provide a set of utilities, our apps, accessible via a
command-line interface.

Each app is represented by a subcommand, which takes some
input parameters. In general, one of them is a path to
a YAML file containing the specification for one execution
of the app. An app delivers family of strings representing
the output of any processing it is designed to perform.
In order to organize those strings, the apps support a templating engine,
`inja`, allowing the user to specify a template which references
the produced family of strings.

## Use the CLI Apps
```
cd build
./ltsy --help
```

### determinize-tt
Examples of YAML inputs:
- https://github.com/greati/logicantsy/blob/master/examples/cli/tt-determinizer-avron-4v.yml

### rule-soundness-checker
Examples of YAML inputs:
- https://github.com/greati/logicantsy/blob/master/examples/cli/sequent-rule-soundness.yml

### axiomatize-monadic-matrix
Examples of YAML inputs:
- Implication-free strong Kleene (one-dimensional): https://github.com/greati/logicantsy/blob/master/examples/cli/mc-pnm-axiomatization-impfree-strong-kleene-1D.yml
- A two-dimensional logic over the billatice FOUR: https://github.com/greati/logicantsy/blob/master/examples/cli/mc-pnm-axiomatization-avron-4v-2D.yml
