Please clone the repository from https://github.com/quixdb/squash

------------------------------

Squash - Compresion Abstraction Library
<https://quixdb.github.io/squash>
======================================================================

Squash is an abstraction library which provides a single API to access
many compression libraries, allowing applications a great deal of
flexibility when choosing a compression algorithm, or allowing a
choice between several of them.

Because Squash provides a single API, any algorithms which Squash has
a plugin to support will be useable from any language for which Squash
provides bindings.

The actual integration with individual compression libraries is done
through plugins which can be installed separately from Squash itself
and are not loaded until they are required. This allows Squash
consumers to utilize a great many compression algorithms without
rewriting code or unnecessary bloat.

For documentation, support, and other details, please see
<http://quixdb.github.io/squash/>.
