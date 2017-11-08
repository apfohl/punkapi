# PunkAPI

This is a client to access data from the [PunkAPI](https://punkapi.com/).

## Prerequisites

- [libcurl](https://curl.haxx.se/libcurl/)
- [jzon](https://github.com/apfohl/jzon/)

## Compilation

For development builds:

    $ make

For release builds:

    $ make release

## Usage

    $ ./punkapi [options]

Without any options `punkapi` will return a list of 25 beers.

**Options**:

- `-r` Return a random beer
- `-p <page>` Return the given page of beers
- `-i <items>` Set the number of beers per page
- `-k` Use insecure connection
