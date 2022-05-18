gztool
======

GZIP files indexer, compressor and data retriever.
Create small indexes for gzipped files and use them for quick and random data extraction.
No more waiting when the end of a 10 GiB gzip is needed!

See [`Installation`](https://github.com/circulosmeos/gztool#installation) for Ubuntu, the [Release page](https://github.com/circulosmeos/gztool/releases) for executables for your platform, and [`Compilation`](https://github.com/circulosmeos/gztool#compilation) in case you want to compile the tool.

Also, a *`magic`* file to correctly identify `gztool`'s index files with linux `file` command is provided: you can append it (or overwrite your empty) `/etc/magic` file or append/copy it to your home directory as `~/.magic` (note the point prepending the name).

Considerations
==============

* Please, note that the initial complete **index creation for an already gzip-compressed file** (`-i`) still consumes as much time as a complete file decompression.   
Once created the index will reduce access time to the gzip file data.

Nonetheless, note that `gztool` **creates index interleaved with extraction of data** (`-b`), so in the practice there's no waste of time. Note that if extraction of data or just index creation are stopped at any moment, `gztool` will reuse the remaining index on the next run over the same data, so time consumption is always minimized.

Also **`gztool` can monitor a growing gzip file** (for example, a log created by rsyslog directly in gzip format) and generate the index on-the-fly, thus reducing in the practice to zero the time of index creation. See the `-S` (*Supervise*) option.

* Index size is about 0.33% of a compressed gzip file size if the index is created **after** the file was compressed, or 10-100 times smaller (just a few Bytes or kiB) if `gztool` itself compresses the data (`-c`).

Note that the size of the index depends on the span between index points on the uncompressed stream - by default it is 10 MiB: this means that when retrieving randomly situated data only 10/2 = 5 MiB of uncompressed data must be decompressed (on average) no matter the size of the gzip file - which is a fairly low value!    
The span between index points can be adjusted with `-s` (*span*) option (the minimum is `-s 1` or 1 MiB).    
For example, a span of `-s 20` will create indexes half the size, and `-s 5` will create indexes twice bigger.


Background
==========

By default gzip-compressed files cannot be accessed in random mode: any byte required at position N requires the complete gzip file to be decompressed from the beginning to the N byte.   
Nonetheless Mark Adler, the author of [zlib](https://github.com/madler/zlib), provided years ago a cryptic file named [zran.c](https://github.com/madler/zlib/blob/master/examples/zran.c) that creates an "index" of "windows" filled with 32 kiB of uncompressed data at different positions along the un/compressed file, which can be used to initialize the zlib library and make it behave as if compressed data begin there.   

`gztool` builds upon zran.c to provide a useful command line tool.    
Also, some optimizations and brand new behaviours have been added:

* `gztool` can correctly read incomplete `gzip`-concatenated-files (using `-p`), that is, a gzip composed of a concatenation of `gzip` files, some of which are not correctly terminated. This can happen, for example, when using [rsyslog's veryRobustZip omfile option](https://www.rsyslog.com/doc/v8-stable/configuration/modules/omfile.html#veryrobustzip) and the process that is logging is abruptly terminated and then restarted.
* `gztool` can store line numbering information in the index (use only if source data is text!), and retrieve data from a specific line number using `-L`. (Using `-[xXz]` when creating the index selects Unix new line format (default), old Mac new line format, or no line information respectively.)
* **`gztool` can *Supervise* an still-growing gzip file** (for example, a log created by rsyslog directly in gzip format) and generate the index on-the-fly, thus reducing in the practice to zero the time of index creation. See `-S`.
* extraction of data and index creation are interleaved, so there's no waste of time for the index creation.
* **index files are reusable**, so they can be stopped at any time and reused and/or completed later.
* an *ex novo* index file format has been created to store the index
* span between index points is raised by default from 1 MiB to 10 MiB, and can be adjusted with `-s` (*span*).
* windows **are compressed** in file
* windows are not loaded in memory unless they're needed, so the application memory footprint is fairly low (< 1 MiB)
* `gztool` can compress files (`-c`) and at the same time generate an index that is about 10-100 times smaller than if the index is generated after the file has already been compressed with gzip.
* **Compatible with [`bgzip` files](http://www.htslib.org/doc/bgzip.html)** (short-uncompressed complete-gzip-block sizes)
* **Compatible with complete `gzip`-concatenated-files** (aka [gzip members](http://tools.ietf.org/html/rfc1952#page-5))
* **Compatible with [rsyslog's veryRobustZip omfile option](https://www.rsyslog.com/doc/v8-stable/configuration/modules/omfile.html#veryrobustzip)** (variable-short-uncompressed complete-gzip-block sizes)
* data can be provided from/to stdin/stdout
* `gztool` can be used to remotely retrieve just a small part of a bigger gzip compressed file and sucessfully decompress it locally. See [this stackexchange thread](https://unix.stackexchange.com/questions/429197/reading-partially-downloaded-gzip-with-an-offset/#541903). Just note that the `gztool` **index file** must be also available.

Installation
============

* `gztool` is available using directly `apt-get install gztool` in [Debian 11 and later](https://packages.debian.org/source/bullseye/gztool) and in [Ubuntu *Groovy Gorilla* (20.10) and above](https://launchpad.net/ubuntu/+source/gztool).

* In Ubuntu, [using my repository](https://launchpad.net/~roberto.s.galende/+archive/ubuntu/gztool):

        sudo add-apt-repository ppa:roberto.s.galende/gztool
        sudo apt-get update
        sudo apt-get install gztool

* See the [Release page](https://github.com/circulosmeos/gztool/releases) for executables for your platform, including **Windows**. If none fit your needs, `gztool` is very easy to compile: see next sections.

Compilation
===========

*zlib.a* archive library is needed in order to compile `gztool`: the package providing it is actually `zlib1g-dev` (this may vary on your system):

    $ sudo apt-get install zlib1g-dev

    $ gcc -O3 -o gztool gztool.c -lz -lm

If you wish you can use autoconf to check the dependencies, build and
test `gztool`:

    $ autoreconf && ./configure && make check

This will produce a binary in `gztool`.

Compilation in Windows
======================

Compilation in Windows is possible using *gcc* for Windows and compiling the original *zlib* code to obtain the needed archive library `libz.a`.    
Please, note that **executables for different platforms are provided** on the [Release page](https://github.com/circulosmeos/gztool/releases).    

* download gcc for Windows: [mingw-w64](https://sourceforge.net/projects/mingw-w64/files/latest/download)
* Install it and add the path for gcc.exe [to your Windows PATH](https://www.computerhope.com/issues/ch000549.htm)
* Download zlib code and compile it with your new gcc: [zlib](https://github.com/madler/zlib/archive/master.zip)
* The previous step generates the file **zlib.a** that you need to compile *gztool*:
Copy gztool.c to the directory where you compiled zlib, and do:
    
    `gcc -static -O3 -I. -o gztool gztool.c libz.a -lm`


Usage
=====

      gztool (v1.4.3)
      GZIP files indexer, compressor and data retriever.
      Create small indexes for gzipped files and use them
      for quick and random-positioned data extraction.
      No more waiting when the end of a 10 GiB gzip is needed!
      //github.com/circulosmeos/gztool (by Roberto S. Galende)

      $ gztool [-[abLnsv] #] [-[1..9]AcCdDeEfFhilpPrRStTwWxXz|u[cCdD]] [-I <INDEX>] <FILE>...

      Note that actions `-bcStT` proceed to an index file creation (if
      none exists) INTERLEAVED with data flow. As data flow and
      index creation occur at the same time there's no waste of time.
      Also you can interrupt actions at any moment and the remaining
      index file will be reused (and completed if necessary) on the
      next gztool run over the same data.

     -[1..9]: Factor of compression to use with `-[c|u[cC]]`, from
         best speed (`-1`) to best compression (`-9`). Default is `-6`.
     -a #: Await # seconds between reads when `-[ST]|Ec`. Default is 4 s.
     -A: Modifier for `-[rR]` to indicate the range of bytes/lines in
         absolute values, instead of the default incremental values.
     -b #: extract data from indicated uncompressed byte position of
         gzip file (creating or reusing an index file) to STDOUT.
         Accepts '0', '0x', and suffixes 'kmgtpe' (^10) 'KMGTPE' (^2).
     -C: always create a 'Complete' index file, ignoring possible errors.
     -c: compress a file like with gzip, creating an index at the same time.
     -d: decompress a file like with gzip.
     -D: do not delete original file when using `-[cd]`.
     -e: if multiple files are indicated, continue on error (if any).
     -E: end processing on first GZIP end of file marker at EOF.
         Nonetheless with `-c`, `-E` waits for more data even at EOF.
     -f: force file overwriting if destination file already exists.
     -F: force index creation/completion first, and then action: if
         `-F` is not used, index is created interleaved with actions.
     -h: print brief help; `-hh` prints this help.
     -i: create index for indicated gzip file (For 'file.gz' the default
         index file name will be 'file.gzi'). This is the default action.
     -I string: index file name will be the indicated string.
     -l: check and list info contained in indicated index file.
         `-ll` and `-lll` increase the level of index checking detail.
     -L #: extract data from indicated uncompressed line position of
         gzip file (creating or reusing an index file) to STDOUT.
         Accepts '0', '0x', and suffixes 'kmgtpe' (^10) 'KMGTPE' (^2).
     -n #: indicates that the first byte on compressed input is #, not 1,
         and so truncated compressed inputs can be used if an index exists.
     -p: indicates that the gzip input stream may be composed of various
         incorrectly terminated GZIP streams, and so then a careful
         Patching of the input may be needed to extract correct data.
     -P: like `-p`, but when used with `-[ST]` implies that checking
         for errors in stream is made as quick as possible as the gzip file
         grows. Warning: this may lead to some errors not being patched.
     -r #: (range): Number of bytes to extract when using `-[bL]`.
         Accepts '0', '0x', and suffixes 'kmgtpe' (^10) 'KMGTPE' (^2).
     -R #: (Range): Number of lines to extract when using `-[bL]`.
         Accepts '0', '0x', and suffixes 'kmgtpe' (^10) 'KMGTPE' (^2).
     -s #: span in uncompressed MiB between index points when
         creating the index. By default is `10`.
     -S: Supervise indicated file: create a growing index,
         for a still-growing gzip file. (`-i` is implicit).
     -t: tail (extract last bytes) to STDOUT on indicated gzip file.
     -T: tail (extract last bytes) to STDOUT on indicated still-growing
         gzip file, and continue Supervising & extracting to STDOUT.
     -u [cCdD]: utility to compress (`-u c`) or decompress (`-u d`)
              zlib-format files to STDOUT. Use `-u C` and `-u D`
              to manage raw compressed files. No index involved.
     -v #: output verbosity: from `0` (none) to `5` (nuts).
         Default is `1` (normal).
     -w: wait for creation if file doesn't exist, when using `-[cdST]`.
     -W: do not Write index to disk. But if one is already available
         read and use it. Useful if the index is still under an `-S` run.
     -x: create index with line number information (win/*nix compatible).
         (Index counts last line even w/o newline char (`wc` does not!)).
         This is implicit unless `-X` or `-z` are indicated.
     -X: like `-x`, but newline character is '\r' (old mac).
     -z: create index without line number information.

      EXAMPLE: Extract data from 1 GiB byte (byte 2^30) on,
      from `myfile.gz` to the file `myfile.txt`. Also gztool will
      create (or reuse, or complete) an index file named `myfile.gzi`:
      $ gztool -b 1G myfile.gz > myfile.txt


Please, **note that STDOUT is used for data extraction** with `-bLtTu` modifiers.

Examples of use
===============

* Make an index for `test.gz`. The index will be named `test.gzi`:

        $ gztool -i test.gz

* Make an index for `test.gz` with name `test.index`, using `-I`:

        $ gztool -I test.index test.gz

* Also `-I` can be used to indicate the complete path to an index in another directory. This way the directory where the gzip file resides could be read-only and the index be created in another read-write path:

        $ gztool -I /tmp/test.gzi test.gz

* Retrieve data from uncompressed byte position 1000000 inside test.gz. An index file will be created **at the same time** (named `test.gzi`):

        $ gztool -b 1m test.gz

* **Supervise an still-growing gzip file and generate the index for it on-the-fly**. The index file name will be `openldap.log.gzi` in this case. `gztool` will execute until interrupted or until the supervised gzip file is overwritten from the beginning (this happens usually with compressed log files when they are rotated). It can also stop at first end-of-gzip data with `-E`.

        $ gztool -S openldap.log.gz

* The previous command can be sent to background and with no verbosity, so we can forget about it:

        $ gztool -v0 -S openldap.log.gz &

* Creating and index for all "\*gz" files in a directory:

        $ gztool -i *gz

        ACTION: Create index

        Index file 'data.gzi' already exists and will be used.
        (Use `-f` to force overwriting.)
        Processing 'data.gz' ...
        Index already complete. Nothing to do.

        Processing 'data_project.2.tar.gz' ...
        Built index with 73 access points.
        Index written to 'data_project.2.tar.gzi'.

        Processing 'project_2.gz' ...
        Built index with 3 access points.
        Index written to 'project_2.gzi'.

* Extract data from `project.gz` byte at 1 GiB to STDOUT, and use `grep` on this output. Index file name will be `project.gzi`:

        $ gztool -b 1G project.gz | grep -i "balance = "

* Please, note that STDOUT is used for data extraction with `-bcdtT` modifiers, so an explicit command line redirection is needed if output is to be stored in a file:

        $ gztool -b 99m project.gz > uncompressed.data

* Extract data from a gzipped file which index is still growing with a `gztool -S` process that is monitoring the (still-growing) gzip file: in this case the use of `-W` will not try to update the index on disk so that the other process is not disturb! (this is so because `gztool` always tries to update the index used if it thinks it's necessary):

        $ gztool -Wb 100k still-growing-gzip-file.gz > mytext

* Extract data from line 10 million, to STDOUT:

        $ gztool -L 10m compressed_text_file.gz

* Nonetheless note that if in the precedent example an index was previously created for the gzip file without the `-x` parameter (or not using `-L`), **as it doesn't contain line numbering info**, `gztool` will complain and stop. This can be circumvented by telling `gztool` to use another new index file name (`-I`), or even not using anyone at all with `-W` (do not write index) and an index file name that doesn't exists (in this case `None` - it won't be created because of `-W`), and so ((just) this time) the gzip will be processed from the beginning:

        $ gztool -L 10m -WI None compressed_text_file.gz

* Extract all data from a [rsyslog's veryRobustZip](https://www.rsyslog.com/doc/v8-stable/configuration/modules/omfile.html#veryrobustzip) that contains dirty data. This *corrupted-gzip-files* can arise when using [rsyslog's veryRobustZip omfile option](https://www.rsyslog.com/doc/v8-stable/configuration/modules/omfile.html#veryrobustzip) and the process that is logging is abruptly terminated and then restarted - this produces an incorrectly-terminated-gzip stream that is followed by another gzip stream **in the same file**. `gzip` (nor `zlib`) cannot read this files beyond the point of error. But `gztool` can correctly extract all data (and only good data) using `-p` (*patch*) parameter:

        $ gztool -p -b0 compressed_text_file.gz

This creates, as usual, the index file `compressed_text_file.gzi`. In order to not create it, `-W` (*do not Write index*) can be used:

        $ gztool -pWb0 compressed_text_file.gz

Note that `-p` can require up to twice the time for decompression, because it performs two decompression processes: the usual one, and another one that is performed **in advance** of the usual and which is the one that detects errors, marks them, and finds new entry points to end/begin the decompression circumventing the problems.

Note also that these *corrupted-gzip-files* should be always decompressed with `-p` parameter, even if a `gztool` index file exists for them, because the index file stores entry points, but does not store where do errors occur in the `gzip` file.
That said, if the `-[bL]` point of extraction is beyond the point(s) of error in the `gzip` file and an index file exists, then the decompression can proceed fine without `-p`, as the index points stored in the index file are always clean.

* When tailing an still-growing gzip file (`-T`) that could contain errors at some point, one may still want to obtain output from the gzip stream as soon as possible - this is what the patching option `-P` is for (like `-p` but capitalized): with `-p` `gztool` decompress the stream about 48 kiB ahead of the output that is actually shown/written in order to catch possible gzip-stream errors ahead of output, and so maintain always a clean output without error-introduced artifacts. This has the side effect that output must always wait for that 48 kiB of data to be available in advance, which if the file grows slowly can take a very long time. With `-P` the buffer-ahead restriction is relaxed to just as few bytes as available before reaching end-of-file and waiting for new data, so responsiveness is as quick as without `-p`. The side effect of `-P` is that depending on the gzip file some errors may lead to incorrect output being shown/written - though in this case a "PATCHING WARNING" would be shown (to stderr).

        $ gztool -PT application_log.gz
        ...
        PATCHING: Gzip stream error found @ 15745693 Byte.
        PATCHING WARNING:
            Part of data extracted after (compressed 15728640 / uncompressed 92783074) Byte
            collides with previously extracted data, after a badly-ended gzip stream
            was found @15745693 and a new starting point began @15700759.
        PATCHING: New valid gzip full flush found @ 15700759 Byte.
        ...

The same applies to `-S` though in this case there's no output, as only the index is being constructed:

        $ gztool -PS application_log.gz
        ...
        PATCHING: Gzip stream error found @ 15745693 Byte.
        PATCHING WARNING:
            Data extracted around the patching point may overlap.
        PATCHING: New valid gzip full flush found @ 15700759 Byte.
        ...


* To tail to stdout, *like a* `tail -f`, an still-growing gzip file (an index file will be created with name `still-growing-gzip-file.gzi` in this case):

        $ gztool -T still-growing-gzip-file.gz

* More on files still being "Supervised" (`-S`) by another `gztool` instance: they can also be tailed *à la* `tail -f` without updating the index on disk using `-W`:

        $ gztool -WT still-growing-gzip-file.gz

* Compress (`-c`) an still growing (`-E`) file: in this case both `still-growing-file.gz` and `still-growing-file.gzi` files will be created *on-the-fly* as the source file grows. Note that in order to terminate compression, Ctrl+C must be used to kill gztool: this results in an incomplete-gzip-file as per GZIP standard, but this is not important as it will contain all the source data, and both `gzip` and `gztool` (or any other tool) can correctly and completely decompress it.

        $ gztool -Ec still-growing-file

* If you have an *incomplete* index file (they just do not have the length of the source data, as it didn't correctly finish) and want to make it complete and so that the length of the uncompressed data be stored, just unconditionally *complete* it with `-C` with a new `-i` run over your gzip file: note that as the existent index data is used (in this case the file `my-incomplete-gzip-data.gzi`), only last compressed bytes are decompressed to complete this action:

        $ gztool -Ci my-incomplete-gzip-data.gz

* Decompress a file like with gzip (`-d`), but do not delete (`-D`) the original one: Decompressed file will be `myfile`. Note that gzipped file **must** have a ".gz" extension or `gztool` will complain.

        $ gztool -Dd myfile.gz

* Decompress a file that does not have ".gz" file extension, like with gzip (`-d`):

        $ cat mycompressedfile | gztool -d > my_uncompressed_file

* Show internals of all index files in this directory. `-e` is used to not stop the process on the first error, if a `*.gzi` file is not a valid gzip index file. The `-ll` list option repetition will show data about each index point. `-lll` also decompress each point's window to ensure index integrity:

        $ gztool -ell *.gzi

            Checking index file 'accounting.gzi' ...
            Size of index file:        184577 Bytes (0.37%/gzip)
            Guessed gzip file name:    'accounting.gz' (66.05%) ( 50172261 Bytes )
            Number of index points:    15
            Size of uncompressed file: 147773440 Bytes
            Compression factor       : 66.05%
            List of points:
            @ compressed/uncompressed byte (index data size in Bytes @window's beginning at index file), ...
            #1: @ 10 / 0 ( 0 @56 ), #2: @ 3059779 / 10495261 ( 13127 @80 ), #3: @ 6418423 / 21210594 ( 6818 @13231 ), #4: @ 9534259 / 31720206 ( 7238 @20073 )...
        ...

If `gztool` finds the gzip file companion of the index file, some statistics are shown, like the index/gzip size ratio, or the ratio of compression of the gzip file. 

Also, if the gzip is complete, the size of the uncompressed data is shown. This number is interesting if the gzip file is bigger than 4 GiB, in which case `gunzip -l` cannot correctly calculate it as it is [limited to a 32 bit counter](https://tools.ietf.org/html/rfc1952#page-5), or if the gzip file is in `bgzip` format, in which case `gunzip -l` would only show data about the first block (< 64 kiB).

* Note that `gztool -l` tries to guess the companion gzip file of the index looking for a file with the same name, but without the `i` of the `.gzi` file name extension, or without the `.gzi`. But the gzip file name can also be directly indicated with this format:

        $ gztool -l -I index_filename gzip_filename

In this latter case only a pair of index+gzip filenames can be indicated with each use.

* Use a truncated gzip file (100000 first bytes are removed: (not zeroed, removed; if they're zeroed cautions are the same, but `-n` is not needed)), to extract from byte 20 MiB, **using a previously generated index**: as far as the `-b` parameter refers to a byte *after* an index point (See `-ll`) and `-n` be less than that needed first index point, this is always possible. In this case *-I gzip_filename.gzi* is implicit:

        $ gztool -n 100001 -b 20M gzip_filename.gz

Take into account that, as shown, the first byte of the truncated `gzip_filename.gz` file is numbered **100001**, that is, the bytes retain the order number in which they appear in the original file (that's the reason why it is not the *1* byte).

Please, note that index point positions at index file **may require also the previous byte** to be available in the truncated gzip file, as a gzip stream is not byte-rounded but a stream of pure bits. Thus **if you're thinking on truncating a gzip file, please do it always at least by one byte before the indicated index point in the gzip** - as said, it may not be needed, but in 7 of 8 cases it is needed.

Index file format
=================

Index files are created by default with extension '.gzi' appended to the original file name of the gzipped file:

    filename.gz     ->     filename.gzi

If the original file doesn't have ".gz" extension, ".gzi" will be appended - for example:

    filename.tgz     ->     filename.tgz.gzi

There's a special header to mark ".gzi" files as index files usable for this app:

    +-----------------+-----------------+
    |   0x0 64 bits   |    "gzipindx"   |     ~     16 bytes = 128 bits
    +-----------------+-----------------+

This is "version 0" header, that is, it does not contain lines information. The header indicating that the index contains lines information is a "version 1" header, differing only in the capital "X" (each index registry point in this case contains an additional 64-bit counter to take lines into account). Next versions (if any) would use "gzipindx" string with lower and capital letters following a binary counting as if they were binary digits.

    +-----------------+-----------------+
    |   0x0 64 bits   |    "gzipindX"   |     version 1 header (index was created with `-[xX]` parameter)
    +-----------------+-----------------+

Note that this header has been built so that this format will be "compatible" with index files generated for *bgzip*-compressed files. **[bgzip](http://www.htslib.org/doc/bgzip.html)** files are totally compatible with gzip: they've just been made so every 64 kiB of uncompressed data the zlib library is restart, so they are composed of independent gzipped blocks one after another. The *bgzip* command can create index files for bgzipped files in less time and with much less space than with this tool as they're already almost random-access-capable. The first 64-bit-long of bgzip files is the count of index pairs that are next, so with this 0x0 header gztool-index-files can be ignored by *bgzip* command and so bgzipped and gzipped files and their indexes could live in the same folder without collision.

All numbers are stored in big-endian byte order (platform independently). Big-endian numbers are easier to interpret than little-endian ones when inspecting them with an hex editor (like `od` for example).

Next, and almost one-to-one pass of *struct access* is serialize to the file. *access->have* and *access->size* are both written even though they'd always be equal. If the index file is generated with `-S` or `-T` on a still-growing gzip file (or somehow the index hasn't been completed because the gzip data was still incomplete), the values on disk for *access->have* and *access->size* will be respectively 0x0..0 and "number of actual index points written" (both uint64_t) to mark this fact. *access->size* MAY be UINT64_MAX to avoid the need to write this value as the number of index points are added to the file: as the index is incremental the number of points can be determined by reading the index until EOF. *access->have* MAY also be greater than zero but lower than *access->size*: this can occur when an already finished index is increased with new points (source gzip may have grown) - in this case this is also considered an incomplete index: when the index be correctly closed both numbers will have the same value (a Ctrl+C before would leave the index "incomplete", but usable for next runs in which it can be finished).

After that, comes all the *struct point* data. As previously said, windows are compressed so a previous register (32 bits) with their length is needed. Note that an index point with a window of size zero is possible.

After all the *struct point* structures data, the original uncompressed data size of the gzipped file is stored (64 bits).

Please note that not all stored numbers are 64-bit long. This is because some counters will always fit in less length. Refer to code.

With 64 bit long numbers, the index could potentially manage files up to 2^64 = 16 EiB (16 777 216 TiB).

#### Line number counting

Regarding line number counting (`-[xX]`), note that gztool's index counts last line in uncompressed data even if the last char isn't a newline char - whilst `wc` command will not count it in this case!. Nonetheless, line counting when extracting data with `-[bLtT]` does follow `wc` convention - this is in order to not obtain different (+/-1) results reading `gztool` output info and `wc` counts.    

Also note that line counting when a gzip file / index file aren't still complete, always starts in **1**. This is coherent with the previous statement, and it's also reasonable because if number counting is activated (`-[xX]`) there'll presumably be lines beautifully ending with a *new line* char (or chars in case of Windows: `CR+LF`) somewhere in the string.    

magic file
==========

A *`magic`* file to correctly identify `gztool`'s index files with linux `file` command is provided: you can append it (or overwrite your empty) `/etc/magic` file or append/copy it to your home directory as `~/.magic` (note the point prepending the name).

Other tools which try to provide random access to gzipped files
===============================================================

* also based on [zlib's `zran.c`](https://github.com/madler/zlib/blob/master/examples/zran.c):

  * Perl module [Gzip::RandomAccess](https://metacpan.org/pod/Gzip::RandomAccess). It seems to create the index only in memory, after decompressing the whole gzip file.
  * Go package [gzran](https://github.com/coreos/gzran). It seems to create the index only in memory, after decompressing the whole gzip file.
  * [jzran](https://code.google.com/archive/p/jzran/). It's a Java library based on the zran.c sample from zlib.

* [*bgzip*](https://github.com/samtools/htslib/blob/develop/bgzip.c) command, available in linux with package *tabix* (used for chromosome handling). This discussion about the implementation is very interesting: [random-access-to-zlib-compressed-files](https://lh3.github.io/2014/07/05/random-access-to-zlib-compressed-files). I've developed also a [`bgztail` command tool](https://github.com/circulosmeos/bgztail) to tail bgzipped files, even as they grow.

* [*dictzip*](http://manpages.ubuntu.com/manpages/bionic/man1/dictzip.1.html) command, is a format compatible with gzip that stores an index in the header of the file. Uncompressed size is limited to 4 GiB - see also `idzip` below. The dictionary header cannot be expanded if more gzip data is added, and it cannot be added to an existent gzip file - both issues are succesfully managed by `gztool`.

* [*idzip*](https://github.com/fidlej/idzip) Python command and function, builds upon *dictzip* to overcome the 4 GiB limit of `dictzip` by using multiple [gzip members](http://tools.ietf.org/html/rfc1952#page-5).

* [*GZinga*](https://tech.ebayinc.com/engineering/gzinga-seekable-and-splittable-gzip/): Seekable and Splittable GZip, provides Java language classes to create gzip-compatible compressed files [using the Z_FULL_FLUSH option](https://tech.ebayinc.com/engineering/gzinga-seekable-and-splittable-gzip/), to later access or split them.

* [indexed_gzip](https://github.com/pauldmccarthy/indexed_gzip) Fast random access of gzip files in Python: it also creates file indexes, but they are not as small and they cannot be reused as easily as with `gztool`.

* [lzopfs](https://github.com/vasi/lzopfs). Random access to compressed files with a FUSE filesystem: allows random access to lzop, gzip, bzip2 and xz files - lzopfs allows to mount gzip, bzip2, lzo, and lzma compressed files for random read-only access. I.e., files can be seeked arbitrarily without a performance penalty. It hasn't been updated for years, but there's a [fork that provides CMake installation](https://github.com/mxmlnkn/lzopfs). See also this [description of its internals](https://stackoverflow.com/questions/4457997/indexing-random-access-to-7zip-7z-archives#23458181).

* [pymzML v2.0](https://pymzml.readthedocs.io/en/latest/index.html). One key feature of pymzML is the ability to write and read indexed gzip files. It has its [own index format](https://pymzml.readthedocs.io/en/latest/index_gzip.html#igzip-file-format-specification). Read also [this](https://watermark.silverchair.com/bty046.pdf).

* [zindex](https://github.com/mattgodbolt/zindex) creates SQLite indexes for text files based on regular expressions.

Other interesting links
=======================

* [decompress-fs](https://github.com/narhen/decompress-fs): A passthrough fuse filesystem that decompresses archieved files on the fly.

* [MiGz](https://engineering.linkedin.com/blog/2019/02/migz-for-compression-and-decompression) for Compression and Decompression, supports multithreaded decompression.

* [parallelgzip](https://github.com/shevek/parallelgzip): pure Java equivalent of the pigz parallel compresssor.

* [pigz](https://zlib.net/pigz/): A parallel implementation of gzip for modern multi-processor, multi-core machines. pigz was written by Mark Adler, and uses the zlib and pthread libraries.

* Article on parallel gzip decompression: [`Parallel decompression of gzip-compressed files and random access to DNA sequences`](https://arxiv.org/pdf/1905.07224.pdf).

Version
=======

This version is **v1.4.3**.

Please, read the *Disclaimer*. In case of any errors, please open an [issue](https://github.com/circulosmeos/gztool/issues).

License
=======

    A work by Roberto S. Galende
    distributed under the same License terms covering
    zlib from Mark Adler (aka Zlib license):
      This software is provided 'as-is', without any express or implied
      warranty.  In no event will the authors be held liable for any damages
      arising from the use of this software.
      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute it
      freely, subject to the following restrictions:
      1. The origin of this software must not be misrepresented; you must not
         claim that you wrote the original software. If you use this software
         in a product, an acknowledgment in the product documentation would be
         appreciated but is not required.
      2. Altered source versions must be plainly marked as such, and must not be
         misrepresented as being the original software.
      3. This notice may not be removed or altered from any source distribution.

    /* zlib.h -- interface of the 'zlib' general purpose compression library
      version 1.2.11, January 15th, 2017
      Copyright (C) 1995-2017 Jean-loup Gailly and Mark Adler
      This software is provided 'as-is', without any express or implied
      warranty.  In no event will the authors be held liable for any damages
      arising from the use of this software.
      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute it
      freely, subject to the following restrictions:
      1. The origin of this software must not be misrepresented; you must not
         claim that you wrote the original software. If you use this software
         in a product, an acknowledgment in the product documentation would be
         appreciated but is not required.
      2. Altered source versions must be plainly marked as such, and must not be
         misrepresented as being the original software.
      3. This notice may not be removed or altered from any source distribution.
      Jean-loup Gailly        Mark Adler
      jloup@gzip.org          madler@alumni.caltech.edu
      The data format used by the zlib library is described by RFCs (Request for
      Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
      (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).
    */

Disclaimer
==========

**This software is provided "as is", without warranty of any kind, express or implied. In no event will the authors be held liable for any damages arising from the use of this software.**

Author
======

by [Roberto S. Galende](loopidle@gmail.com)   

on code by Mark Adler's [zlib](https://github.com/madler/zlib).
