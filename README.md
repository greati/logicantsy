# logicantsy

Library and CLI assistant for research in Logic.

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
### tt-axiomatizer

#### Output

```json
"tables": {
    "connective1": table1,
    "connective2": table2,
    ...
    "connectiveM": tableM
},
"axiomatizations": {
    "connective1": [
        [A11, B11, C11, D11],
        ...
        [A1N1, B1N1, C1N1, D1N1]   
    ],
    "connective2": [...],
    ...
    "connectiveM": [...]
}
```
- Each Aij, Bij, Cij, Dij is a string of formulas, separated by a comma.
- Each `tablei` is a string representing a truth table

### tt-determinizer
### rule-soundness-checker
