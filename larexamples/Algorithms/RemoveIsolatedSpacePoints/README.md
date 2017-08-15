| Example name:   | RemoveIsolatedSpacePoints             |
| --------------- | ------------------------------------- |
| Type:           | LArSoft algorithm with module         |
| Author:         | Gianluca Petrillo (petrillo@fnal.gov) |
| Created on:     | June 30, 2016                         |
| Version:        | 1.1                                   |



#  A simple algorithm example: `RemoveIsolatedSpacePoints`  ####################



This example shows an algorithm filtering a collection of input objects,
complete with a module using it.

Features of the algorithm and module:

* the interface is a single operation function
* reads the configuration from a FHiCL file
* provides a template FHiCL configuration

Missing features that you need to go elsewhere if you need to implement:

* data product associations
* module reaction to framework events (new event, end of run...)
* `lar_ci` integration test

Technical choices:

* employs the module/algorithm factorisation model
* uses validation of the configuration
* has tests
* it is fully documented via Doxygen interpreted comments


### How to use this document                                                 ###

This document is pretty long. You are encouraged to read it all, but you might
understandably opt out of that.
To make it even longer, we assume that you are aware of the existence of the
_art_ introductory documentation (in particular, the _art_ workbook). Having
read it will imbue the present text with a lot more sense. And if you have no
idea about what _art_ is, or what a _art_ module is, you definitely need to go
there first. Also, bits of information that are already present in LArSoft
example `AtomicNumber` are not repeated in their entirety here.

This document is organised in sections. We suggest that you read the section
above to know what this example is about, and, below, the (very terse)
explanation of the model and the beginning of the section of the
[Algorithm task and design choices].
These contain information about the design that can stimulate your own.
Then, you can dig into the code and come back to the pertinent section when you
have questions.

For every questions, answered or not here, you are strongly encouraged to
contact the example's author (contact information is at the top of this file).

And, if you want to have a bit more printer friendly format, know this text file
is written in markdown format and you can convert it to something else with:
    
    pandoc -s -S --toc -o README.html README.md
    pandoc -s -S --toc -o README.pdf README.md
    
et cetera.



## The factorisation model for algorithms  #####################################

The factorisation model prescribes the code to be split in a functional part
that is independent of the framework, and a framework interface.

Here we call the functional part solving a given problem simply _algorithm_.
Its _art_ framework interface is an _art_ module.

A more extensive description of this model can be found in [LArSoft wiki].



## Files  ######################################################################

The example is split between two directories:

~~~~
larexamples/Algoritmhs/RemoveIsolatedSpacePoints/    ## contains example code ##
|-- README                                                          # this file
|-- PointIsolationAlg.h                           # generic isolation algorithm
|-- SpacePartition.h            # container used by PointIsolationAlg algorithm
|-- SpacePointIsolationAlg.h    # header for the space point specific algorithm
|-- SpacePointIsolationAlg.cxx  # source for the space point specific algorithm
|-- RemoveIsolatedSpacePoints_module.cc                  # art module interface
`-- removeisolatedspacepoints_standard.fcl       # example module configuration

test/Algoritmhs/RemoveIsolatedSpacePoints/           ## contains example test ##
|-- SpacePointTestUtils.h        # header with common utilities for these tests
|-- SpacePointTestUtils.cxx      # source with common utilities for these tests
|-- PointIsolationAlg_test.cc    # a simple unit test for the generic algorithm
|-- PointIsolationAlgRandom_test.cc # other unit test for the generic algorithm
|-- PointIsolationAlgStress_test.cc   # a stress test for the generic algorithm 
|-- point_isolation_test.fcl          # configuration of the test of art module
|-- SpacePointMaker_module.cc                     # module producing test input
`-- CheckDataProductSize_module.cc                # module checking test output
~~~~

Each directory also has its own `CMakeLists.txt` file.

LArSoft mildly recommends the following file name suffices:

* `.h` for all the header files
* `.cc` for modules, services and tests; this is because CET build macros prefer
  the `.cc` suffix
* `.cxx` for algorithm implementation and anything not connected to _art_
 
If adding code to an existing directory, it's good practise to spend a couple of
minutes to observe the name patterns of files in the directory, its parent and
its siblings, and adapt to them.


### Directory structure                                                      ###

The directory structure in this example is minimal, with a code section under
the repository code tree (`larexamples`) and a test section under the proper
tree (`test`).
Compared with the service examples, this structure is simpler.
A goal of the factorisation is to have the algorithm library independent of the
framework. Here we obtain that effect by a careful tuning of _cmake_.
A consequence is that not all the code in the directory compiles in an
out-of-framework environment. If that is a requirement, the code should be split
into two separate directories as in the service examples.
It is possible that the repository where you are going to commit your code
already has such a structure, in which case you will likely be asked to adapt to
it, and it is possible that your algorithm will share a directory with others.

The two disjoint subtrees replicate the same internal structure. This is a
LArSoft prescription: all the code and configuration files are in `larexamples`
subdirectory, and the tests are under the `test` subdirectory.



## The algorithm  ##############################################################

In this section the algorithm purpose, design choices and implementation are
described. The next section will then describe its framework interface, the
_art_ module.


### Algorithm task and design choices                                        ###

The algorithm is expected to receive as input a collection of LArSoft space
points (`recob::SpacePoint`) and to return the information of which of them are
isolated.

Our design is to split the tasks as follows:

* the module reads the configuration and the space points, and delivers them
  in addition to the required services to the algorithm
* the algorithm processes the input space points and returns the index of the
  ones that are not isolated
* the module converts the list of indices in a new data product with copies of
  the space points that are not isolated, and saves it into the event

The decision of letting the module take care of converting the list of indices
into an actual data product instead of having the algorithm return the new
collection directly is driven by two reasons:

1. the task of converting the indices into space points is pretty trivial;
2. another module or algorithm could be using the algorithm to extract the
   non-isolated space points without the need of a full copy: for example,
   they might use a mask, or a list of pointers to the original space points,
   or the index list directly.

In general, the life of the algorithm is expected to be as follows:

1. creation
2. configuration
3. acquisition of input
4. execution
5. delivery of output
6. clean up
7. (back to 1., 2. or 3.)

where the creation and configuration are often merged and expected to happen
rarely (possibly only once), while the remaining points happen many times.

The algorithm is simple enough that it can receive all its input in a single
method call, and its return value is easily expressed in a return value.
Therefore, we chose to have a *"stateless" algorithm* (except for the
configuration). This means that no information is cached between public method
calls. For example, between the configuration call and the algorithm execution
call, the algorithm object has no input stored, and immediately after the
execution call has returned, the algorithm has already forgotten both
input points and result. This pattern is *very much recommended*, coming
with advantages like:

* no need to "clear" results or inputs; we have seen cases in LArSoft where this
  was both required and omitted, and memory waste would ensue
* the same algorithm instance can be used multiple times, at the same time; this
  is pure blessing in a multi-threaded job
* this model tends to reduce duplication of data, as a common alternative
  pattern is to keep the result in the algorithm and return _a copy_ of it

If you have hard times making this model fit your algorithm, please ask for
support: it may be worth it.

There is another decision that we took in design phase. The isolation algorithm,
in its core, processes just 3D coordinates. LArSoft space points are a glorified
form of 3D points, and the algorithm does not require any of the additional
glory. In order to accommodate for the chance of the isolation algorithm to be
used in other contexts, the core algorithm is written for a generic data type,
and the algorithm dealing with space points will take care of providing the
necessary degree of translation.
This decision brings a good deal of complication. This complication can be
avoided altogether by having the algorithm deal only with space points, which is
also a legitimate choice.


### The generic algorithm: `PointIsolationAlg`                               ###

This algorithm expects as configuration:

* a world box, describing the coordinate range that the points live in
* a isolation radius

This algorithm expects as input:

* an iterator to the first point, and an iterator past the last one; or a
  container supporting the C++14 `std::cbegin()`/`std::cend()` idiom
  (all C++ standard containers do)

The algorithm returns:

* a vector of indices of the non-isolated points; the index corresponding to
  the point pointed by the first iterator is `0`

The design choices drive us to have an interface including:

* creation and configuration
* execution (including input, execution and output delivery)

Clean up is not necessary in a stateless algorithm.

The choices of this algorithm are toward the "low level": no automatic checks,
no automatic frills, and assume the caller knows how to use the algorithm.

The configuration is set by passing a full configuration object. 

This implementation goes very far in trying to be generic. The type of
coordinate may be either `double` or single precision (`float`), and we choose
to have it as a template argument.
Also we don't constrain at all the data type holding the coordinate information,
leaving it as another template argument (this is another instance where a
stateless algorithm matters, since the input type appears only as parameter in
the relevant methods rather than in the whole class).
There is a catch, though: our algorithm will need to know how to extract the
information it needs from this unspecified `Point` data type. So it assumes that
a set of helper functions exist that take a `Point` object and return a
coordinate value. This set of functions is documented as `PositionExtractor`,
and an implementation for a `Point` that is a pointer to an array of coordinates
(x, y and z in sequence) is also provided.
This technique is used in other libraries. It combines flexibility of
implementation, no execution overhead with any decent compiler, and some serious
non-readability. The former will be illustrated in the space-point specific
algorithm implementation.

One aspect the algorithm does not try to be generic about is which actual
algorithm to use: no run-time polymorphism is used. Introducing it would require
to drop the generic iterator interface and would likely come with a price in 
speed. Whatever the solution, methods in "tight" loops (like `closeEnough()`)
should not be virtual: in this way, the function can be "inlined", that is,
included in the calling function without the processor having to jump to another
place to get it, and code locality is an important optimisation factor in modern
processors.

The algorithm uses a container to organise the input points. This container is
segregated in its own header, `SpacePartition.h`. While we don't necessarily
want one header per class, having complex, self-standing classes in their own
file is a good practice. The `SpacePartition` class could prove to be useful in
other contexts, and it will be easier to reuse if in a separated file.

Note that the algorithm uses basically only standard C++ libraries (the use of
`cet::sum_squares()` might be very easily replaced) and has no _art_ dependency.
It _does_ have a LArSoft dependency, in the form of a utility library
`GridContainers.h` (currently in `lardata` repository). This header, and the
ones it includes, do not introduce additional dependencies as they are pure C++
and pure header libraries (in fact, they were created for this example, and
promoted to general utility code later).


#### The configuration data structure

The configuration class is a simple structure. This is usually more than enough.
In this case, though, some of the data members are also custom classes
(`Range_t`). The class `Range_t` is very straightforward, and we have just added
some convenience methods to it.


#### Construction and configuration

Configuration is achieved by passing a full configuration object.
The only constructor requires one, and `reconfigure()` method allows to change
the configuration after the algorithm is instantiated.
In this way, the algorithm is always configured.

Partial reconfiguration is not allowed, e.g. changing isolation radius without
changing the containment box.

On the other end, there is no validation in the current implementation.
The configuration _can_ be validated with a separate call, but it's up to the
caller to decide whether to do that by calling `validateConfiguration()` on
a configuration object.


#### The algorithm

The main algorithm receives the input from two iterators.
This choice makes it easier to use it with different container types.
We provide also a version of the method taking the collection directly.
This relies on the collection supporting `std::cbegin()` and `std::cend()`: this
techniques is similar to the one we used with `PositionExtractor`, but it comes
directly from the C++ standard library.

A possible, basic implementation of the algorithm is brute force, trying all
combinations and then decide. While this scales very badly with input data size,
it is easier to get right. Such an algorithm is available in a method, to be
used as cross check implementation for unit tests with arbitrary content.

The limit of the brute force approach is that it compares each of the _N_ points
in the input with each other of the still _N_ points: this is a _N_ x _N_
algorithm, that means doubling the input quadruplicates the time taken.
The main implementation of the example algorithm tries to reduce the number of
comparisons. The idea is simple: we can partition the input points in space
cells, and then compare cells first, as most of them might be far enough that
the points in one can't affect the isolation of the points in the other.
We have a container that arranges points by cells (that is `SpacePartition`
class), then we define for each cell which neighbouring cells _could_ render a
point in one cell non-isolated, and then we will compare only points in those
cells. The size of the cell is a critical choice to have the algorithm
efficient. Here, with not too much of a thinking, we chose it so that if there
is more than one point in it, all the points in the cell are non-isolated.
We buy execution speed with the use of additional memory to rearrange the input;
if the cell size is chosen too small, memory usage will explode (it increases as
the _cube_ of _N_). So we have a rough estimation of the basic memory required
for the optimal cell size, and if that is too much, we increase the cell size.
The algorithm will not be able to rely on the assumption that more points in
a single cells mean they are not all isolated, and it will be slower.

The `closeEnough()` method hides all the coordinate extraction.
It employs `PositionExtractor` via some helper functions that have the advantage
they don't need the template type explicitly specified --- the compiler guesses
it. Also note the use of `cet::sum_of_squares()` for convenience, and that we
compare distance squares to avoid expensive square root evaluations.

The algorithm can be improved in many ways. Some of them are marked as "TODO" in
the code, for future developers to consider and implement them.
With some additional knowledge, it might be possible also to back up to just
use the brute force approach, that may be faster with few points in input.


#### Documentation

The documentation of the algorithm includes an example of usage and an
explanation of the configuration. These are required in order to have a usable
algorithm.

The favourite documentation language is Doxygen, that is the documentation
closest to the code and therefore the easiest to be maintained together with it.
This can be hard to get right. If you want to test how your documentation looks
like, you can quickly create a Doxygen configuration file:

    cd "${MRB_SOURCE}/larexamples/larexamples/Algorithms/RemoveIsolatedSpacePoints"
    doxygen -c

and then edit the lines:

    PROJECT_NAME = "RemoveIsolatedSpacePoints"
    RECURSIVE    = YES

You can now run it:

    doxygen Doxyfile

You can take a look at the result starting from `html/index.html`, navigating to
the "Classes" section and to `PointIsolationAlg` class.

Doxygen produces a lot of messages of undocumented objects. Parsing this output
feels like an endless effort, but it's very likely to catch many typos.


### The space-point specific algorithm: `SpacePointIsolationAlg`             ###

The space-point specific algorithm uses LArSoft specific knowledge (and
additional dependencies) to apply the `PointIsolationAlg` algorithm to LArSoft's
data structure.

The main point of this algorithm is to call `PointIsolationAlg`, and its
structure revolves on the latter's. The documentation again shows how to use the
algorithm, highlighting the construction, set up and execution phases.

One task of the implementation is to extract from the geometry description the
total size of the volume to be covered. This volume is defined as a box
including the volume from all TPC's in the detector (across all cryostats).
This approach can be nefarious in case of DUNE Far Detector, where the cryostats
are very far from each other. A more sensible approach would be to treat the
cryostats independently.

The type `Coord_t` is written in a C++11-heavy way and it translates into
`double`. That definition in fact reads the type directly from the type of
_x_ coordinate of `recob::SpacePoint`. Ideally, `recob::SpacePoint` itself would
expose the type of its coordinate directly by defining a type name.


#### Configuration

Configuration happens only on construction, which is served in two flavours:
with configuration from a `Config` data structure, or via a FHiCL parameter set.
In the end, the former is always used, and it implements FHiCL configuration
validation. Documentation on FHiCL validation can be found in
[_fhiclcpp_ wiki][fhiclcpp]. In short, if the FHiCL parameter set that generates
the `Config` object has unknown parameters or has mandatory parameters missing,
an exception will be automatically thrown.

Note that on construction, no `PointIsolationAlg` is created, since its creation
requires a complete configuration, and that will not be available until the
point containment box is known.
For this reason, the base algorithm object can't be stored as a data member.
We have it as a pointer, initially invalid.


#### Set up

In the set up phase, the algorithm acquires geometry information.
Without knowing better, we assume that the box where the points are living is
a box large enough to include all TPC volumes. A better algorithm could attempt
to detect that volume once it has the input points available, and also treat
cryostats as separate and independent entities.

Internally, initialisation is performed by the `initialize()` method.
It finally creates a fully configured algorithm on the first call, and on the
following ones it takes advantage of `PointIsolationAlg` ability to be
reconfigured. It also uses the facility that the base algorithm provides to
validate the values of the configuration, although currently a misconfiguration
can arise only from a bug.


#### Execution

The interface for execution phase is similar to the one from the base algorithm.
A method accepting any pair of iterators pointing to `recob::SpacePoint` is
offered. Another method accepts a `std::vector<recob::SpacePoint>`, which is the
most common collection of space points expected in LArSoft data files.

We have explicitly encoded the requirement that the iterator must point to
`recob::SpacePoint` (or a derived class). While this is not technically required
now, supporting a more general case might complicate future extensions where
that case can't be supported any more (for example, if the new algorithm uses
the `Chi2()` method to remove badly reconstructed points, or the uncertainty to
treat the space points like entities with physical extension).
The check is a `static_cast` which will fail at compilation time, or never
again.

No protection is provided against incomplete set up. We rely on users to read
the documentation and call the set up function. Until this happens, no base
algorithm is instantiated and any attempt to run will result in unpredictable
behaviour (actually, very likely a segmentation violation).


#### Bridge to the base algorithm

The base algorithm has as requirement that its input data must support the
`PositionExtractor` interface. Since `recob::SpacePoint` does not "naturally" do
so, we provide a specialisation `PositionExtractor<recob::SpacePoint>`.

This specialisation extracts the position from `recob::SpacePoint::XYZ()`.
No additional code is expected to be generated in a optimised build, and the
final result is that `PointIsolationAlg::closeEnough()` method will be coded as

~~~~{.cpp}
return cet::sum_of_squares(
  A.fXYZ[0] - B.fXYZ[0], A.fXYZ[1] - B.fXYZ[1], A.fXYZ[2] - B.fXYZ[2])
  <= config.radius2;
~~~~

and actually, `cet::sum_of_squares()` and `closeEnough()` itself can disappear
and be inlined into `PointIsolationAlg::removeIsolatedPoints()`. An exceedingly
smart compiler might then use special optimisations for the resulting loop.

This inlining would not be possible if polymorphic were involved in
`recob::SpacePoint::XYZ()` or `PointIsolationAlg::closeEnough()` or
`PositionExtractor`.



## The _art_ module: `RemoveIsolatedSpacePoints`  ##############################

The task of the _art_ producer module is to act as an interface between the
algorithm, `SpacePointIsolationAlg`, and the _art_ framework.

The module accepts as an input a collection of space points and produces a new
collection of space points. An alternative approach might be to have in produce
a vector of `art::Ptr` to the original space points, which may save a bit of
space on disk (`recob::SpacePoint` uses, at the time of this writing, 84 bytes).

The algorithm is contained by the module, since at the time the module is
constructed all the information needed to instantiate the algorithm is
available.


### Configuration                                                            ###

The module also uses the FHiCL configuration validation facilities. Monir
details differ respect to the use in the algorithm.
The configuration object contains two elements: an input tag that specifies the
input data product label, and the full configuration of the algorithm,
encapsulated info a `fhicl::Table` member. This is how a nested parameter set
is implemented with configuration validation.

To enable the validation, _art_ module constructor interface accepts a special
FHiCL table object, `art::EDProducer::Table<Config>` (which is not that special
after all, being like a `fhicl::Table<Config>`, plus the information that some
special keys like `module_type` should be ignored for validation purposes).
If the type of that object is defined in the class a `Parameters` type, _art_
will be also able to print information about the module itself. Try it out:

~~~~
lar --print-description RemoveIsolatedSpacePoints
~~~~

More information on these features is available in the [_art_ wiki][art wiki].

The resulting configuration can be seen in the example FHiCL configuration:
`removeisolatedspacepoints_standard.fcl`. That is not a very usable
configuration, since there are two parameters and both need to be overwritten
before the configuration is accepted (that is the purpose of assigning them a
`@nil` value), but it works as a template.
If we had more likely parameters, we would use them. Unfortunately, a
sensible isolation ratio depends on the wire pitch, and the name of the input
collection is just unpredictable.


### Construction                                                             ###

The module constructor is almost trivial.
The data members are initialised from the configuration.
A lot is going on to configure the algorithm, but it is well contained in a
seemingly normal initialisation.

The only other task of a producer constructor is to declare what it is going to
produce. In this case, it's a collection (`vector`) of `recob::SpacePoint`.
Producers don't even have to call the base class constructor, so we are done.


### Execution                                                                ###

The _art_ framework will call `produce()` method on each new event.
Here we have to run the algorithm, that implies setting it up, feeding it with
input and use the result to produce module's output.

The input is a collection of space points whose label was specified in the
configuration. Here we read it by a `art::Event::getValidHandle()`, which
ensures the data product is present in the input.

The algorithm set up requires a geometry service provider, that we obtain from
the framework and immediately pass to the algorithm.
In principle this action might be moved to the constructor, if we know that
the geometry will not change. On a new run LArSoft geometry _can_ change, hence
our choice.

We run the algorithm and save the result in a new variable (that allows an
important optimisation, preventing a copy from the return value of the method
into our variable). Finally, we build a new collection of space points managed
by a `std::unique_ptr`, which is required by _art_ to move data products around.

A terse information is printed, that is hopefully not too long as to result
annoying, but it shows that the module has run and also allows for detecting
blatant misconfigurations by eye: it's unlikely that there are no isolated
points, and even more unlikely that all of them are.


### Artisms                                                                  ###

The module source file ends with a call to _art_'s `DEFINE_ART_MODULE` macro.
This is a necessary piece for the _art_ framework. This and many other secrets
of the business of writing modules and producers are well described in the _art_
workbook documentation.


### `CMakeLists.txt`                                                         ###

`CMakeLists.txt` instructs cmake about what to do to compile and install the
software in this directory.
We use `art_make()` to provide directions. The use here is fairly simple, but it
deserves a bit of explanation.

One purpose of this structure is to have an algorithm library that is
framework-independent and a framework interface library.
These have naturally different dependencies, the latter including all of the
former and some more.
With `art_make()` we fill two link lists, one for the modules in the source
directory (only one in this case), and one for a library containing all the
source code that is not a module (nor a service).
The latter, introduced by the `LIB_LIBRARIES` keyword, includes also some
LArSoft dependencies, that should be framework-independent too.
The other list, `MODULE_LIBRARIES`, shows to depend on the algorithm library
(that is, `larexamples_Algorithms_RemoveIsolatedSpacePoints`), and some more,
including LArSoft services and _art_ libraries.

Some hints about how to fill this list are given in the `README` file in LArSoft
`AtomicNumber` service example.

Our examples follow a non-standard and not recommended practice: we install
`CMakeLists.txt` files and FHiCL files with `install_source()`. This is done
only because this is an example that should be distributed complete.
Normally, FHiCL files are installed in the correct location thanks to the
`install_fhicl()` directive, and `CMakeLists.txt` files are not installed at
all.


## Tests  ######################################################################

The purpose of these "unit" tests is to exercise all the features of the
algorithm and its _art_ interface.

The test strategy for this algorithm is to have:

* a test verifying the correctness of the algorithm result
* a stress test processing a large data set
* a test exercising the module and verifying the correctness of the result

Since we chose to separate the algorithm in a generic and a space-point specific
part, a reasonable addition would have a correctness test for each of them.
Here we decided to entrust the check of the space-point specific algorithm to
the module test.


test/Algoritmhs/RemoveIsolatedSpacePoints/           ## contains example test ##
|-- SpacePointTestUtils.h        # header with common utilities for these tests
|-- SpacePointTestUtils.cxx      # source with common utilities for these tests
|-- PointIsolationAlg_test.cc    # a simple unit test for the generic algorithm
|-- PointIsolationAlgStress_test.cc   # a stress test for the generic algorithm 
|-- point_isolation_test.fcl          # configuration of the test of art module
|-- SpacePointMaker_module.cc                     # module producing test input
`-- CheckDataProductSize_module.cc                # module checking test output


### Test utility library                                                     ###

Some tasks are, or may be, common to many tests. For example, the creation of
a new `recob::SpacePoint` at a given location, or the population of space with
points according to a pattern.
This small library collects functions to perform those common tasks.
Being they a set of loosely related functionality, the documentation of the
header exposes a short summary of those functionalities.


### `PointIsolationAlg_test`                                                 ###

This unit test processes a few simple point populations.
The test is split in two sections, whose meaning is documented in the test.

This unit test uses the Boost unit test library, that offers support for result
report with fine granularity, and for test debugging.
Running the test executable with `--help` argument will show many options we can
use to figure out where a test failed, or what is broken in the test.

We need some "Boost magic" at the top, that is to define a value for the
`BOOST_TEST_MODULE` preprocessor symbol, and then we include the right headers.
The Boost unit test libraries can be used in many different ways. The one we
choose is possibly the simplest. The test executable will execute some "test
cases", each of them will have a sequence of checks. Each check can be
individually reported on failure. A test case will fail when any of its checks
fail, and the test fails when any of its test cases fail.
A drawback is that the Boost unit test framework extensively employs
preprocessor macros, that makes it very hard to understand some compilation
error messages.

Each test case is enclosed in a `BOOST_AUTO_TEST_CASE` macro. This is more
Boost magic, thanks to which the unit test framework knows a new test case is
present ("AUTO" refers to the registration of this function as a test, that is
not explicit but "automatic"). We leave the content of these functions bare,
having them call another function, which contains the actual test.
In each of the functions, we prepare the input data, we predict the expected
result, we set up the algorithm, we run it and we check the result.
This type of tests is also useful as an example of how to use the algorithm.

Digging into the first test case (by the exhaustive name `PointIsolationTest1`),
we have a single configuration for the algorithm (fixed isolation radius), but
we are going to progressively augment the input data.
The four sections have all the same structure: add input point, modify the list
of expected result, that is the indices of the points we expect non-isolated,
run the algorithm and check the result. The check is performed by a Boost macro,
`BOOST_CHECK_EQUAL_COLLECTIONS`: if the check fails, the Boost unit test
framework will report the failure in a detailed way. Since the isolation
algorithm does not specify the index order in the result, we need to sort both
the expected and the actual result before comparing them.

The second test case (`PointIsolationTest2`) has fixed input data, but varies
isolation radius on each algorithm run. The input data is constructed so that we
can predict the result as function of the radius. We start with a radius that
should include all the points, and then halve it for a number of times.
After each halving, more and more points at the beginning of the input data,
which are the most peripheral ones, become isolated. The procedure is similar
to the previous test case, except that instead of changing the input data we
modify the algorithm configuration, hence testing the `reconfigure()` method.
The check on result size is effectively redundant, but it speeds up finding the
problem in the error log. The check is in a loop, and the potential error
message will show the line number of the failing test, but it will not inform us
about which iteration was ongoing. We make that information promptly available
by asking the test framework to deliver a message on each iteration. The syntax
of `BOOST_TEST_MESSAGE` arguments is not correct C++, and it works because
`BOOST_TEST_MESSAGE` is a preprocessor macro, that is another chapter of the
Boost magics book.


### `PointIsolationAlgRandom_test`                                           ###

This test runs the algorithm on some randomly generated data point
distributions.
There are two sensitive aspects in this kind of tests.

Foremost, it's hard to predict the result of an algorithm on random input. In
fact, another general algorithm would be needed, and then that one is in general
as questionable as ours.
We have such a second algorithm, that was unit-tested and is therefore
reasonably safe. The reason why this is not the main algorithm is that it takes
much longer to run in the general case, and this is a price our test will have
to pay. Incidentally, this tests proves that this "brute force" algorithm is not
always slower than the main one.

The second aspect is that a test needs to be reproducible in order to provide
the chance of debugging on failure. Therefore, we allow to specify the random
seed, and we always print it on the screen.

Technically, the test is similar to `PointIsolationAlg_test`.
It performs tests on two input data set sizes and four different isolation
radii, running the two versions of the algorithm and comparing their result each
time. It also provides timing information on screen for each algorithm
execution.

One technical difference in the test implementation is the use of a "fixture"
class, that provides the Boost test cases with an "environment" (if the fixture
is global, that environment is shared among them). The member variables of the
fixture class are directly accessible in the test cases, and our fixture has
two, the usual `argc` / `argv` pair, which are obtained from the Boost unit test
framework and stored locally.

As an anecdote, this test was added as an after-tough while updating this
document just before release. And it discovered a bug in the algorithm that was
not exposed by any of the other tests, which have a much more controlled set up.


### `PointIsolationAlgStress_test`                                           ###

The stress test runs the algorithm once with a large data set, and reports the
time elapsed to create the data set and to run the algorithm. This test is meant
in part as a development tool, in part as a way to help verify that changes in
the algorithm do not compromise execution speed. For that reason, command line
arguments are used to tune the test execution.

The test follows the usual pattern of input preparation, algorithm configuration
and execution, and verification of the result.
The pattern used for the input sports a symmetry so that either all points are
isolated, or all points are not. The check on the result is just on the number
of non-isolated points found by the algorithm.
Due to the simple nature of the check itself, in addition to the use of command
line arguments, we decided to drop Boost test framework for this test.
Configuration and test failures will yield a termination with an error code.

Due to the motivation of this test as a stress for the algorithm, it also
provides some execution timing information. This is not extremely reliable since
it is based on the "wall clock", but it can report only the time elapsed during
the execution of the algorithm. Using a command line tool like `time` to run the
test will include also loading and test preparation time in the report, which is
typically very short, but will report the "CPU time", that excludes slow downs
due to other processes running on the same machine.


### Test of the _art_ module                                                 ###

The main purpose of this test is to verify that the _art_ interface to the
algorithm is operating as expected. The correctness of the algorithm is a less
important goal, since specific tests already verify it.

The test of the module follows the standard path of each test: create a known
input, run the algorithm and verify the output.
In this context, the algorithm is actually a full _art_ module, and the input
data must be provided either by an input file or by another module.
The validity check needs also to be performed by another module.

Another approach is not to test the module but rather just test the algorithm
it uses. This approach is feasible when the module is extremely simple and does
not contain any algorithmic part, as for example adapting the input or the
output from and to different format. This example is borderline in that there is
in fact some adaptation of the output format, but it's very simple.
In the end, we go the harder way here for demonstration purpose.

The configuration file `point_isolation_test.fcl` describes the test:

* `test: [ createInput, looseIsolTest, tightIsolTest ]`: the first module
  produces the input data , then two instances of the to be tested are executed
  with that same input data
* `check: [ checkLooseIsol, checkTightIsol ]`: verification of the output of the
  two modules

The input creation module, `SpacePointMaker`, is a simple producer that uses a
routine from the space point test library to create a distribution of points on
a grid, similar to the one of the stress test except that the spacing of the
points is variable and the size of the containing volume is determined by the
whole detector. This is a good, if simple, example of a producer, featuring
configuration validation and some documentation.

The verification module, `CheckDataProductSize`, is a analyser module that reads
a collection of space points and, depending on the configuration, checks that
it has the same size as another collection of space points, or a fixed size.
In fact, rather than loading the module with prediction logic, we rely on the
test designer to predict and code in the configuration the expected result.
Our tests are set up so that either no or all points are isolated, and the
configuration of the two analyser instances reflect that.

On failure, the test will raise an exception.


### `CMakeLists.txt`                                                         ###

We need to enumerate all the three tests in the `CMakeLists.txt` file.
We use `cet_test` macro from CET build tools library.

The first test is the correctness test. It is a self-standing executable that
uses Boost unit test library. The enabling line is:

~~~~
cet_test(PointIsolationAlg_test USE_BOOST_UNIT)
~~~~

This instructs _cmake_ to enlist a new test named `PointIsolationAlg_test`,
compiling it from `PointIsolationAlg_test.cc` (default choice when no `SOURCES`
keyword is specified), linking it with Boost, and with no other library.
The test will be run with no arguments (just `PointIsolationAlg_test`) and it is
expected to succeed.

The stress test is at the bottom of the file. Respect to the correctness test,
we don't ask for Boost unit test library, and we specify command line arguments:
`PointIsolationAlgStress_test 10000 0.05`. These were tuned so that with a
debugging build it takes bearable time, on the order of a few seconds.

The module test is more articulate, requiring two additional modules and a
configuration file.
The test itself is again declared with `cet_test` macro, but the executable is
now `lar` (`TEST_EXEC`), and the arguments are the usual ones for a LArSoft run.
In addition, the test needs to find the configuration file in its directory
(`DATAFILES`). Finally, we don't need to build the test executable: `lar` is
already provided (`HANDBUILT`). We do need the modules invoked from the
configuration file to be built, though. The main module, the one subject of the
test, is already built in the source section of the example. The two additional
modules, specific to the test, need to be built here, which we do with the usual
`art_make` macro. Also, there is some library code that the modules use (and
a space-point algorithm unit test might use as well), and `art_make` will build
that too. We just need to direct it not to put in the library the files that are
the source of the other tests: `art_make` would compile into the library all the
sources in the directory. Finally, note that the library compiled from this
directory will be called `test_Algorithms_RemoveIsolatedSpacePoints`, and it is
explicitly mentioned in the link list of the modules.
In principle, the two modules have very different link needs: the checking
module does not use services nor algorithms nor test libraries. We could have
compiled it aside with

~~~~
single_plugin(CheckDataProductSize "module"
    lardata_RecoBase
    art_Framework_Core
    art_Framework_Principal
    art_Utilities
    ${MF_MESSAGELOGGER}
    ${MF_UTILITIES}
    ${FHICLCPP}
  )
~~~~

This is a good idea for a regular module. In this case, we are being a bit
sloppier because this is just a test.

Finally, we install the source code for the tests. This is not always advisable:
in this case, the package is meant as an example, and the test is integral part
of that example. So we install its sources, and we also add `CMakeLists.txt` and
all FHiCL files.


## Questions?  #################################################################

If you have any question about the example, please contact its author.
This section will be populated with questions and their answers.


## Change log  #################################################################

Version 1.0: May 26, 2016 (petrillo@fnal.gov)
  original version

Version 1.1: June 30, 2016 (petrillo@fnal.gov)
  changed the algorithm implementation from a brute-force, try-every-combination
  approach to one that spends memory to reduce the number of comparisons

Version 1.11: August 15, 2017 (seligman@nevis.columbia.edu)
  Teeny change: filename README -> README.md since this is a Markdown file.


[LArSoft wiki]:
  <https://cdcvs.fnal.gov/redmine/projects/larsoft/wiki/Writing_LArSoft_algorithms>
[fhiclcpp]:
  <https://cdcvs.fnal.gov/redmine/projects/fhicl-cpp/wiki/Configuration_validation_and_fhiclcpp_types>
[art wiki]:
  <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Configuration_validation_and_description>
