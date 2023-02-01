# logicantsy

Prototypical library and CLI assistant for research in logic.

## Requirements
```
- C++17
- make, cmake
- bison
- flex
- boost
```

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
### axiomatize-monadic-matrix

Use this app to generate an axiomatization for a monadic two-dimensional (partial) non-deterministic matrix
and search for proof over this axiomatization. The GIF below illustrates this app. Check the example input file [here](https://raw.githubusercontent.com/greati/logicantsy/master/examples/cli/mc-pnm-axiomatization-impfree-strong-kleene-2D.yml).

![grab-landing-page](https://github.com/greati/logicantsy/blob/master/examples/cli/kleene-2d.gif)

Examples of YAML inputs:
- Strong Kleene (two-dimensional): https://raw.githubusercontent.com/greati/logicantsy/master/examples/axiomatization/Kleenebmatrix.yaml
- A two-dimensional logic over the billatice FOUR: https://raw.githubusercontent.com/greati/logicantsy/master/examples/axiomatization/FDEbmatrix.yaml
- mCi in a two-dimensional environment: https://raw.githubusercontent.com/greati/logicantsy/master/examples/axiomatization/mCibmatrix.yaml

Example of a command:
`./ltsy axiomatize-monadic-matrix -f ../examples/axiomatization/Kleenebmatrix.yaml -o latex -s sk2d.tex`

# Project main structure
- `/docs`: documentation
- `/sources/include`
    - `/cli`: stores all code related to the CLI application
    - `/apps`: stores the code for the applications, like the axiomatization algorithm
    - `/core`: all core code
        - `syntax.h`: code related to connectives, signatures and formulas
        - `common.h`, `utils.h`: pieces of code of interest to other pieces of code
        - `exception.h`: any exception code goes here
        - `/combinatorics`: code related to combinatorics tasks, like generating all subsets of another set
        - `/exception`: folder holding codes for exception (yes, this *must* be unified with `exception.h` in some moment)
        - `parser`: stores code for parsing formulas and other entities (only formulas for now)
        - `printers`: code that attempts to separate the printing of entities from their logic
        - `proof-theory`: code related to proof-theory concepts
        - `semantics`: code related to semantics, like truth-tables and generalized matrices
- `/sources/tests`: here you may find how to use the classes

# Usages in research

- Vitor Greati's MSc thesis: introduction of the theoretical aspects behind the
tool and presentation of the first finite axiomatization of the logic **mCi**.
See [this](https://github.com/greati/logicantsy/blob/be58fcf0df32a253526d422403777a9975a61bd5/examples/outputs/mcicons.pdf).
- C. Caleiro, S. Marcelino, P. Filipe, M. Cristani:
analytic calculus for conjunctions, disjunctions, negations and contraries of intersective gradable adjectives when uttered by a single (see [this](https://github.com/greati/logicantsy/blob/db6c76179992cb9292a8c620e8be3a1427ee9d3e/examples/outputs/inters_grad_adjectives_single.pdf)) or more agents (see [this](https://github.com/greati/logicantsy/blob/db6c76179992cb9292a8c620e8be3a1427ee9d3e/examples/outputs/inters_grad_adjectives_multiple.pdf)).

# Future of logicantsy

The present implementation is a prototype written in the context of my MSc thesis for some functionalities
that will be part of a much more complete and integrated tool, which will be
developed in partnership with researchers of the Instituto de Telecomunicações (Portugal) in the near future.
