<p align="center"><a href="http://www.fix8.org"><img src="http://fix8.org/fix8/fix8_Logo_RGB.png"></a></p>

# [Fix8](http://www.fix8.org) Open Source C++ FIX Engine

A modern open source C++ FIX framework featuring complete schema driven customisation, high performance and fast application development.

The system is comprised of a compiler for generating C++ message and field encoders,
decoders and instantiation tables; a runtime library to support the generated code
and framework; and a set of complete client/server test applications.

[Fix8 Market Tech](http://www.fix8mt.com/) develops and maintains Fix8 and [Fix8Pro](http://www.fix8pro.com), the commercially supported version of Fix8.

## Contents

1. [Contents](#contents)
1. [Features](#features)
1. [Directory Layout](#directory-layout)
1. [Documentation](#documentation)
1. [Branch Layout](#branch-layout)
1. [C++11](#c11)
1. [External Dependencies (required)](#external-dependencies-required)
1. [Optional Dependencies](#optional-dependencies)
1. [Building on Linux/UNIX](#building-on-linuxunix)
1. [Building on OSX](#building-on-osx)
1. [Building on Windows](#building-on-windows)
1. [Support](#support)
1. [Downloads](#downloads)
1. [Getting help or reporting problems](#getting-help-or-reporting-problems)
1. [Making a Pull Request](#making-a-pull-request)
1. [License](#license)
1. [Fix8Pro and Fix8 Market Technologies](#fix8pro-and-fix8-market-technologies)
1. [More Information](#more-information)

## Features

* [Fix8](http://www.fix8.org) helps you get your [FIX protocol](http://www.fixprotocol.org/) client or server up and running quickly. Using one of the standard FIX schemas you can have a FIX client or server up and running in next to no time.

* Statically compile your FIX xml schema and quickly build your FIX application on top. If you need to add customised messages or fields, simply update the schema and recompile.

* Fix8 is the fastest C++ Open Source FIX framework. Our testing shows that Fix8 is on average 68% faster encoding/decoding the same message than Quickfix. See [Performance](http://fix8.org/performance.html) to see how we substantiate this shameless bragging.

* Fix8 supports standard `FIX4.X` to `FIX5.X` and `FIXT1.X`. If you have a custom FIX variant Fix8 can use that too. New FIX versions will be supported.

* Fix8 offers message recycling and a meta-data aware test harness. Incorporates lock free queues, atomics and many other modern techniques.

* Fix8 contains a built-in unit test framework that's being continually revised and extended. Fix8 also has a metadata driven test harness that can be scripted to support captured or canned data playback.

* Fix8 is a complete C++ FIX framework, with client/server session and connection classes (including SSL); support for the standard FIX field types; FIX printer, async logger, async message persister and XML configuration classes.

* Leverage standard components. Fix8 optionally uses industry recognised components for many important functions, including Poco, TBB, Redis, Memcached, BerkeleyDB, Fastflow, Google Test, Google Performance Tools, Doxygen and more. We didn't reinvent the wheel.

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
             <td><code>./</code></td>
             <td>root directory with configure</td>
          </tr>
          <tr>
             <td><code>m4/</code></td>
             <td>additional m4 macros needed by configure</td>
          </tr>
          <tr>
             <td><code>compiler/</code></td>
             <td>the f8c compiler source</td>
          </tr>
          <tr>
             <td><code>contrib/</code></td>
             <td>user contributed files</td>
          </tr>
          <tr>
             <td><code>doc/</code></td>
             <td>Fix8 documentation</td>
          </tr>
          <tr>
             <td><code>doc/man</code></td>
             <td>manpages for Fix8 utilities</td>
          </tr>
          <tr>
             <td><code>doc/html</code></td>
             <td>doxygen documentation (optionally generated when built)</td>
          </tr>
          <tr>
             <td><code>include/</code></td>
             <td>header files for the runtime library and compiler</td>
          </tr>
          <tr>
             <td><code>include/ff/</code></td>
             <td>header files for FastFlow</td>
          </tr>
          <tr>
             <td><code>runtime/</code></td>
             <td>runtime library source</td>
          </tr>
          <tr>
             <td><code>stocklib/</code></td>
             <td>stock FIX library builds</td>
          </tr>
          <tr>
             <td><code>util/</code></td>
             <td>additional utilities source</td>
          </tr>
          <tr>
             <td><code>msvc/</code></td>
             <td>Microsoft Visual Studio project files</td>
          </tr>
          <tr>
             <td><code>pro/</code></td>
             <td>Fix8Pro extensions (commercial version only)</td>
          </tr>
          <tr>
             <td><code>schema/</code></td>
             <td>quickfix FIX xml schemas</td>
          </tr>
          <tr>
             <td><code>test/</code></td>
             <td>test applications client/server source</td>
          </tr>
          <tr>
             <td><code>utests/</code></td>
             <td>unit test applications</td>
          </tr>
    </tbody>
</table>


## Documentation

See our [Wiki](https://fix8engine.atlassian.net/wiki) for detailed help on using Fix8. Access to this documentation is free but will require
a login. For our complete API Documentation see [here](http://fix8.org/fix8/html/). All the source code is self-documenting using doxygen.

## Branch Layout

<table>
    <thead>
         <tr>
            <th>Branch</th>
            <th>github path</th>
            <th>Description</th>
         </tr>
    </thead>
    <tbody>
          <tr>
             <td><pre>master</pre></td>
             <td>https://github.com/fix8/fix8/tree/master</td>
             <td>This is the default branch. All stable releases are made here.</td>
          </tr>
          <tr>
             <td><pre>dev</pre></td>
             <td>https://github.com/fix8/fix8/tree/dev</td>
             <td>This is the development stream and is updated continually. Features and bug fixes scheduled for release are developed and tested here.</td>
          </tr>
          <tr>
             <td><pre>dev-premain</pre></td>
             <td>https://github.com/fix8/fix8/tree/dev-premain</td>
             <td>This branch is used to marshall development changes that are ready for release. When significant changes are made to the dev branch, this branch will be used to keep other changes separate.</td>
          </tr>
          <tr>
             <td><pre>gh-pages</pre></td>
             <td>https://github.com/fix8/fix8/tree/gh-pages</td>
             <td>This branch contains the static html for the Fix8 website.</td>
          </tr>
    </tbody>
</table>


## C++11

Fix8 now **requires C++11 compiler support**. Fix8 will refuse to build without it. If you are using clang or gcc make sure you have the

	-std=c++11

flag on your compiler command line. Some older compiler versions may no longer be supported. Sorry.

## External Dependencies (required)

Fix8 requires the following third-party software (header files and
libraries) being installed to build properly:

- Poco C++ Libraries [basic edition](http://pocoproject.org/download/index.html)

Additional libraries are needed for building on Windows, [see here](https://fix8engine.atlassian.net/wiki/x/EICW).

## Optional Dependencies

You can either choose the internally supplied [Fastflow](http://calvados.di.unipi.it/dokuwiki/doku.php?id=ffnamespace:about) or use...

- Intel Threading Building Blocks [OSS edition](http://threadingbuildingblocks.org/download)

If you wish to use the built-in unit tests (recommended):

- [googletest](https://code.google.com/p/googletest/downloads/list)

If you wish to use tcmalloc (recommended):

- [gperftools](https://code.google.com/p/gperftools/downloads/list)

If you wish to build the html documentation, you will need:

- [Doxygen](http://www.doxygen.org)

If you wish to use Redis for message persistence:

- [hiredis](https://github.com/redis/hiredis)

If you wish to use libmemcached for message persistence:

- [libmemcached](http://libmemcached.org/libMemcached.html)

If you wish to use BerkeleyDB for message persistence:

- [Berkeley DB C++](http://www.oracle.com/technetwork/products/berkeleydb/downloads/index.html)

## Building on Linux/UNIX

The build system is based on automake/autoconf/libtool.
You **must** have [libtool](http://www.gnu.org/software/libtool/) installed to build.

	% tar xvzf 1.3.4.tar.gz
	% cd fix8-1.3.4
	% ./bootstrap
	% ./configure
	% make
	% make install

If you have built the test cases, you can also run them as follows:

	% make check

## Building on OSX

You **must** have [glibtool, autotools](http://www.jattcode.com/installing-autoconf-automake-libtool-on-mac-osx-mountain-lion/) installed to build.

	% tar xvzf 1.3.4.tar.gz
	% cd fix8-1.3.4
	% export LIBTOOLIZE=`which glibtoolize`
	% ./bootstrap
	% ./configure
	% make
	% make install

Please see [this document](https://fix8engine.atlassian.net/wiki/x/B4AtAQ) for more instructions for building on OSX.

## Building on Windows

Please see [this document](https://fix8engine.atlassian.net/wiki/x/EICW) for detailed instructions for building on Windows.

## Support

Please refer to the following pages for help:
- [FAQ](http://fix8.org/faq.html)
- [Fix8 Support Group](https://groups.google.com/forum/#!forum/fix8-support)
- [Fix8 Developer Group](https://groups.google.com/forum/#!forum/fix8-developer)
- [API Documentation](http://fix8.org/fix8/html)
- [Jira Issues Page](https://fix8engine.atlassian.net/)
- [Fix8 News](http://blog.fix8.org/)
- [Wiki](https://fix8engine.atlassian.net/wiki)
- [Twitter](https://twitter.com/fix8engine)

## Downloads

Please refer to the following page:
- [Downloads](http://fix8.org/downloads.html)

## Getting help or reporting problems

1. Review the topics on the **[Fix8 Support Group](https://groups.google.com/forum/#!forum/fix8-support)** and
the **[Fix8 Developer Group](https://groups.google.com/forum/#!forum/fix8-developer)**.
If you cannot find any help there **create a new topic and ask the support group for advice.**

1. *Do not* email us directly. **Support questions sent directly to us will be redirected to the support group.**

1. *Do not* post the same question to both fix8-support and fix8-developer groups.

1. If you are considering submitting a problem report, make sure you have identified a **potential problem with Fix8 and not a problem with your application**.
These aren't the same thing. So, for example, if your application is crashing, there are many possible causes and some will relate
to your build, your code and your configuration and will *not be a problem with the framework implementation*. Make sure you have eliminated
these possibilities and that you have reviewed topics in the [Fix8 Support Group](https://groups.google.com/forum/#!forum/fix8-support) and
the [Fix8 Developer Group](https://groups.google.com/forum/#!forum/fix8-developer) *before* submitting a problem report.

1. If you believe you have found a problem that needs fixing, **go to the [Jira Issues Page](https://fix8engine.atlassian.net/),
register and create an issue.** Select 'fix8' as the project and provide *as much detail as possible*. Attach supporting files and extracts, like logfiles,
stack traces, sample configuruation files, config.log, etc.

1. If you have already implemented a fix, and wish to make a pull request on Github, *create an issue in Jira first*.
This will help us track the problem and ensure that the solution is properly tested.

Also, consider the following:

- We also provide commercial support and help. See [below](#fix8pro-and-fix8-market-technologies).

- We welcome genuine problem reports and encourage users to help us improve the product - for you and with your help.

- If you are on [LinkedIn](http://linkedin.com) join the LinkedIn group **Fix8 Users and Developers**
for more help and information about the Fix8 project.

## Making a Pull Request

If you want to submit a change to the repository, it needs to be *made on the dev branch*. The following instructions may help:

1. Login to Jira, create a ticket for your changes, describing in detail the bug fix or improvement
1. Login to github
1. Create a fork of fix8
1. If you are using the command line git, clone your fork and choose the dev branch<br><code>% git clone https://github.com/[your_repo_name]/fix8.git -b dev</code>
1. Make your changes to this branch
1. Submit changes to your branch and push the branch to your fork
1. Create a pull request at fix8:dev
1. Wait for your pull request to be accepted to fix8:dev
1. Update your fork with recent fix8:dev

## License

Fix8 is released under the [GNU LESSER GENERAL PUBLIC LICENSE Version 3](http://www.gnu.org/licenses/lgpl.html).
See [License](http://fix8.org/faq.html#license) for more information.

## Fix8Pro and Fix8 Market Technologies

[Fix8Pro](http://www.fix8pro.com) is the commercially supported version of Fix8. [Fix8 Market Technologies](http://www.fix8mt.com/)
(Fix8MT) develops and maintains both Fix8Pro and the Fix8 open source versions.
Fix8MT has developers in Australia, China, Canada and the Russian Federation as well as partners in Australia, Japan and India.
Fix8MT is responsible for providing and managing additional support and consultancy services, and works closely with the
Fix8 open source community and partners to grow commercial support services through 3rd party ISVs.

For more information about Fix8Pro see the [Fix8Pro website.](http://www.fix8pro.com)

## More Information

For more information, see the [Fix8 website.](http://www.fix8.org)
