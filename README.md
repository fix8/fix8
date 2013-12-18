<p align="center"><a href="http://www.fix8.org"><img src="http://fix8.org/fix8/fix8_Logo_RGB.png"></a></p>

# [Fix8](http://www.fix8.org) Open Source C++ FIX Engine

A modern open source C++ FIX framework featuring complete schema driven customisation, high performance and fast application development.

The system is comprised of a compiler for generating C++ message and field encoders,
decoders and instantiation tables; a runtime library to support the generated code
and framework; and a set of complete client/server test applications.

## Features

* [Fix8](http://www.fix8.org) helps you get your [FIX protocol](http://www.fixprotocol.org/) client or server up and running quickly. Using one of the standard FIX schemas you can have a FIX client or server up and running in next to no time.

* Statically compile your FIX xml schema and quickly build your FIX application on top. If you need to add customised messages or fields, simply update the schema and recompile.

* Fix8 is the fastest C++ Open Source FIX framework. Our testing shows that Fix8 is on average 68% faster encoding/decoding the same message than Quickfix. See [Performance](http://fix8.org/performance.html) to see how we substantiate this shameless bragging.

* Fix8 supports standard `FIX4.X` to `FIX5.X` and `FIXT1.X`. If you have a custom FIX variant Fix8 can use that too. New FIX versions will be supported.

* Fix8 offers message recycling and a meta-data aware test harness. Incorporates lock free queues, atomics and many other modern techniques.

* Fix8 contains a built-in unit test framework that's being continually revised and extended. Fix8 also has a metadata driven test harness that can be scripted to support captured or canned data playback.

* Fix8 is a complete C++ FIX framework, with client/server session and connection classes (including SSL); support for the standard FIX field types; FIX printer, async logger, async message persister and XML configuration classes.

* Fix8 statically supports nested components and groups to any depth. The Fix8 compiler and runtime library takes the pain out of using repeating groups.

* Fix8 applications are fast. On production level hardware, client NewOrderSingle encode latency is now 2.1us, and ExecutionReport decode 3.2us. Without the framework overhead, NewOrderSingle encode latency is 1.4us. This is being continually improved.

* Fix8 has been designed to be extended, customised or enhanced. If you have special requirements, Fix8 provides a flexible platform to develop your application on.

* Fix8 supports field and value domain validation, mandatory/optional field assertion, field ordering, well-formedness testing, retransmission and standard session semantics.

* Fix8 runs under industry standard Linux on IA32, x86-64, Itanium, PowerPC and ARMv7. It also now runs on *Windows* and *OSX*. Other \*NIX variants may work too.

## Directory Layout

<table>
    <thead>
         <tr>
            <th>Directory</th>
            <th>Description</th>
          </tr>
    </thead>
    <tbody>
          <tr>
             <td>./</td>
             <td>root directory with configure</td>
          </tr>
          <tr>
             <td>m4/</td>
             <td>additional m4 macros needed by configure</td>
          </tr>
          <tr>
             <td>compiler/</td>
             <td>the f8c compiler source</td>
          </tr>
          <tr>
             <td>doc/</td>
             <td>Fix8 documentation</td>
          </tr>
          <tr>
             <td>doc/man</td>
             <td>manpages for Fix8 utilities</td>
          </tr>
          <tr>
             <td>doc/html</td>
             <td>doxygen documentation (optionally generated when built)</td>
          </tr>
          <tr>
             <td>include/</td>
             <td>header files for the runtime library and compiler</td>
          </tr>
          <tr>
             <td>include/ff/</td>
             <td>header files for FastFlow</td>
          </tr>
          <tr>
             <td>runtime/</td>
             <td>runtime library source</td>
          </tr>
          <tr>
             <td>util/</td>
             <td>additional utilities source</td>
          </tr>
          <tr>
             <td>msvc/</td>
             <td>Microsoft Visual Studio project files</td>
          </tr>
          <tr>
             <td>schema/</td>
             <td>quickfix FIX xml schemas</td>
          </tr>
          <tr>
             <td>test/</td>
             <td>test applications client/server source</td>
          </tr>
          <tr>
             <td>utests/</td>
             <td>unit test applications</td>
          </tr>
    </tbody>
</table>


## Documentation

Documentation is available at [API Documentation](http://fix8.org/fix8/html/). All the source code is self-documenting using doxygen.

## External Dependencies (required)

Fix8 requires the following third-party software (header files and
libraries) being installed to build properly:

- Poco C++ Libraries [basic edition](http://pocoproject.org/download/index.html)

Additional libraries are needed for building on Windows, [see here](https://fix8engine.atlassian.net/wiki/display/FX/Building).

## Optional Dependencies

You can either choose the internally supplied [Fastflow](http://calvados.di.unipi.it/dokuwiki/doku.php?id=ffnamespace:about) or use...

- Intel Threading Building Blocks [OSS edition](http://threadingbuildingblocks.org/download)

If you wish to use the built-in unit tests (recommended):

- [googletest](https://code.google.com/p/googletest/downloads/list)

If you wish to use tcmalloc (recommended):

- [gperftools](https://code.google.com/p/gperftools/downloads/list)

If you wish to build the html documentation, you will need:

- [Doxygen](http://www.doxygen.org)

If you wish to use BerkeleyDB for message persistence:

- [Berkeley DB C++](http://www.oracle.com/technetwork/products/berkeleydb/downloads/index.html)

## Building on Linux/UNIX/OSX

The build system is based on automake/autoconf/libtool.
You **must** have [libtool](http://www.gnu.org/software/libtool/) installed to build.

	% tar xvzf 1.0.0-RC3.tar.gz
	% cd fix8-1.0.0-RC3
	% ./bootstrap
	% ./configure
	% make
	% make install

If you have built the test cases, you can also run them as follows:

	% make check

## Building on Windows

Please see [this document](https://fix8engine.atlassian.net/wiki/display/FX/Building) for detailed instructions for building on Windows.

## Support

Please refer to the following pages for help:
- [FAQ](http://fix8.org/faq.html)
- [Fix8 support group](https://groups.google.com/forum/#!forum/fix8-support)
- [Fix8 developer group](https://groups.google.com/forum/#!forum/fix8-developer)
- [API Documentation](http://fix8.org/fix8/html)
- [Jira Issues page](https://fix8engine.atlassian.net/)
- [Fix8 News](http://blog.fix8.org/)

## License

Fix8 is released under the [GNU LESSER GENERAL PUBLIC LICENSE Version 3](http://www.gnu.org/licenses/lgpl.html).
See [License](http://fix8.org/faq.html#license) for more information.

## More Information

For more information, see the [Fix8 website.](http://www.fix8.org)
