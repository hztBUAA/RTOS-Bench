# Building the documentation locally

The documentation is available in pdf and html, and it is generated in the `docs` folder.

## Dependencies

- [Doxygen](doxygen.nl) 1.9.1+
- Make
- Dot tool, part of the graphviz package
- git
- sed

## Build targets

There are several make target that correspond to different documentation formats:
- `html`: Will generate a static website under the ``docs/html`` folder. For ease of use, the site home page will be linked in ``docs/Documentation.html``.
- `pdf`: Will generate a pdf file optimized for being read from a screen.
- ``print``: Will generate a pdf file optimized for being printed in A4 format.
- `clean`: Will remove the generated documentation files.

To use other targets, `make` must be invoked from the `docs` directory, the default target is `html`.
