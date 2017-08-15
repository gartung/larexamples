| Example name:   | ShowerCalibrationGalore               |
| --------------- | ------------------------------------- |
| Type:           | LArSoft service and provider          |
| Author:         | Gianluca Petrillo (petrillo@fnal.gov) |
| Created on:     | April 26, 2016                        |



#  A service with multiple implementations: `ShowerCalibrationGaloreService`  ##


This example shows a complete service with a small abstract interface and
multiple implementations.

Features of the service:

* the interface returns a energy calibration constant, parametrised
* two implementations are provided, one of which must be chosen at run time


Missing features that you need to look for examples elsewhere:

* reaction to framework events (new event, end of run, closing input file...)
* integration test

Technical choices:

* employs the service/provider factorisation model
* uses validation of the configuration
* it is fully documented via Doxygen interpreted comments
* both provider implementations support LArSoft test utilities to easily set up
  this provider in unit tests


This documentation assumes some prerequisite knowledge that can be found in
other LArSoft examples:

* the "factorisation model" for LArSoft service [see example `AtomicNumber`]
* documentation tips [see example `AtomicNumber`]
* namespace explanation [see example `AtomicNumber`]
* FHiCL parmeter validation [see example `AtomicNumber`]
* some code practices [see example `AtomicNumber`]
* constructs in `CMakeLists.txt` files [see example `AtomicNumber`]


### How to use this document                                                 ###

This document is pretty long. You are encouraged to read it all, but you might
understandably opt out of that.
The document is organised in sections. We suggest that you read the section
above to know what this example is about, and the (very terse) explanation of
the model, and the beginning of the section of the provider header.
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


## Design of the interface  ####################################################

To be clear: the design of the interface **is** the hardest part of creating a
service. More so if it is expected to accommodate multiple implementations, some
of which can't be even imagined at the time of design.

This example applies to services that _allow_ multiple implementations. When
designing a new service and its interface, you should consider very carefully
the possibility of some other person or experiment needing a radically different
implementation, and avoid choices that gratuitously make that harder.

When such a service enters LArSoft, it needs to come with an implementation that
is usable by every experiment. The reason is that algorithms using that service
will require it to be present and configured, and not having the service running
precludes from using those algorithms. While it is necessary to have a standard
implementation, that implementation does not necessarily have to be _useful_ to
every experiment. In this example, one implementation reads the calibration as
a single uniform factor. An experiment that does not want yet to bother with
calibration can set up this implementation to always return a neutral factor 1.
This may well be the default for a Monte Carlo simulation, if the energy is
reconstructed with no bias there.


## The factorisation model for services with multiple implementations  #########

The factorisation model prescribes the code to be split in a functional part
that is independent of the framework, and a framework interface.

A more extensive description of this model can be found in LArSoft wiki:
<https://cdcvs.fnal.gov/redmine/projects/larsoft/wiki/Writing_LArSoft_service>.
The example `AtomicNumber` illustrates the model for a single implementation
service.

As a reminder, services with multiple implementations are natively supported by
the _art_ framework. The implementations are C++ classes required to derive from
a common service interface class. User code interacts exclusively with the
interface and it does not know anything about any implementation. The
implementation is only mentioned in the service configuration, where it is
selected with a `service_provider` parameter.

> There is an unfortunate term conflict between what _art_ calls a service
> provider (that is, an implementation of a service allowing multiple
> implementations) and what LArSoft calls a service provider (that is, a
> separate, framework independent class that implements the core service
> functionality).
> 
> On top of this, LArSoft calls a _art_ service interface a class that works as
> a _art_ service and manages a (LArSoft-flavoured) service provider. Within
> _art_, a service interface might mean the interface of a service with multiple
> implementations allowed.
>
> Yes, it's confusing.

In case of services with multiple implementations, each of the implementations
individually follows the pattern of a factorised service.
In addition, all service implementation headers must derive from the common
_art_ service interface (this is a _art_ requirement), and all the provider
headers must derive from a common provider interface (this is a LArSoft
requirement).

This is, admittedly, some degree of complication the service designer must cope
with. The following diagram illustrates the components of this example: we have
a provider interface `ShowerCalibrationGalore` that is implemented in a provider
with a uniform scale factor (`ShowerCalibrationGaloreScale`), and in another one
that relies on external data (`ShowerCalibrationGaloreFromPID`).
Each of them comes with its own _art_ service:

~~~~
                         ,-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~.
                  .......| ShowerCalibrationGaloreService |......
                  :      `-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~'     :
                  :                      |                      :
                  :                      |                      :
,-------------------------------------.  |  ,---------------------------------------.
| ShowerCalibrationGaloreScaleService |  |  | ShowerCalibrationGaloreFromPIDService |
`-------------------------------------'  |  `---------------------------------------'
                  |                      |                      |
                  |        ,-~-~-~-~-~-~-~-~-~-~-~-~-.          |
                  |   .....| ShowerCalibrationGalore |......    |
                  |   :    `-~-~-~-~-~-~-~-~-~-~-~-~-'     :    |
                  |   :                                    :    |
   ,------------------------------.            ,--------------------------------.
   | ShowerCalibrationGaloreScale |            | ShowerCalibrationGaloreFromPID |
   `------------------------------'            `--------------------------------'
~~~~

The dotted lines show inheritance, while the continuous lines show ownership.
Each continuous line groups two classes that form a complete factorised service.
The two boxes with the wiggled lines are special in that they do not have actual
code (they are pure abstract classes with no source file beside the header).
This is, in general, recommended for interface classes.


> We can hear you thinking: do services need to have silly names?
> 
> Answer: no! This is a privilege reserved to our official examples. The reason
> is that we don't want to burn a good service name (e.g., `ShowerCalibration`)
> for an example. But if your service gets to be in LArSoft, that comes with
> the right and duty of a pertinent name.



## Files  ######################################################################

The example is split between two directories:

~~~~
larexamples/Services/ShowerCalibrationGalore/  ### contains all example code ###
|-- README                                                          # this file
|-- Providers                                       ## service provider code ##
|   |-- ShowerCalibrationGalore.h                         # provider interface
|   |-- ShowerCalibrationGaloreFromPID.h   # header of provider implementation
|   |-- ShowerCalibrationGaloreFromPID.cxx # source of provider implementation
|   |-- ShowerCalibrationGaloreFromPIDTestHelpers.h    # helper code for tests
|   |-- ShowerCalibrationGaloreScale.h     # header of provider implementation
|   |-- ShowerCalibrationGaloreScale.cxx   # source of provider implementation
|   |-- ShowerCalibrationGaloreScaleTestHelpers.h      # helper code for tests
|   `-- showercalibrationgalore_standard.fcl # standard provider configuration
`-- Services                                             ## art service code ##
    |-- ShowerCalibrationGaloreService.h                   # service interface
    |-- ShowerCalibrationGaloreFromPIDService.h # header of service implement.
    |-- ShowerCalibrationGaloreFromPIDService_service.cc  # ... and its source
    |-- ShowerCalibrationGaloreScaleService.h   # header of service implement.
    `-- ShowerCalibrationGaloreScaleService_service.cc    # ... and its source
test/Services/ShowerCalibrationGalore/        ### contains all example tests ###
|-- Providers                             ## unit tests for service provider ##
|   |-- CreateTestShowerCalibrationFromPID.cxx    # library: create test input
|   |-- CreateTestShowerCalibrationFromPID.h      # library: create test input
|   |-- CreateTestShowerCalibrationFromPID_test.cc     # test of library above
|   |-- ShowerCalibrationGaloreTests.h              # library: a test function
|   |-- ShowerCalibrationGaloreFromPID_test.cc  # test provider implementation
|   `-- ShowerCalibrationGaloreScale_test.cc    # test provider implementation
`-- Services                          ## unit test for art service interface ##
    |-- ShowerCalibrationGaloreTest_module.cc                   # test service
    |-- test_shower_calibration_from_pid.fcl    # test with one implementation
    `-- test_shower_calibration_scale.fcl       # test with one implementation
~~~~
Each directory also has its own `CMakeLists.txt` file.

LArSoft naming convention is that:

* services and providers are contained in a file with a matching name
* provider interface name is simple and essential (a good one in a non-example
  context might be `ShowerCalibration`)
* provider implementation names should be easy to connect to the interface name
* service names reflect their providers, with `Service` appended or inserted


### Directory structure                                                      ###

The code follows the LArSoft prescription that demands sources split into a main
subtree containing all the code and configuration files, and into another one
containing the tests: see the comments on directory structure in `AromicNumber`
example.

Internally, the subdirectory `ShowerCalibrationGalore` contains two directories.
One contains all the provider-related content, including the provider interface,
the provider implementations, and their configuration.
The other contains all the _art_ service-related content.
Each directory is compiled into its own library.

Additional implementations of providers and services do not have to be in these
same directories, but they should still not be compiled into the same library.


## The service provider interface: `ShowerCalibrationGalore`  ##################

The service provider interface is the fundamental class of the service.
**Algorithms that require shower calibration will see only this interface.**

It is important to design this interface well, because every time it needs to be
expanded or modified, all provider implementations have to be updated
accordingly, across all the experiments that use it.
Unfortunately, there is no sure recipe to get there.

The return value should be a correction factor. For any science purpose, the
correction must also include an uncertainty. The uncertainty may be expensive to
compute, though, and there may be cases where it is of no use.
So we include two hooks, for the computation of the correction factor only, and
for the complete correction with uncertainty. The uncertainty is probably made
of multiple contributions, and it's conceivable that studies would isolate and
treat separately those various contributions. A possible extension of this
interface may consist in the addition of a bit mask argument specifying which
contributions to the total uncertainty to include. From there, one could also
plan for a error matrix. In this example, instead, we stick to just the total
uncertainty.

Here we considered that shower calibration correction may depend on the shower
energy and direction, but also on number of hits, charge profile and section,
etc. It is not easy to put all this in a mandatory interface.
So we decided that it would be the task for the calibration object to extract
the relevant information from the reconstructed shower itself.
The correction might be invoked later in the event analysis, when more is known
about it. For example, a particle identification might be available.
The best candidate for this other argument is again a LArSoft representation of
the particle ID. At the time of writing this example (LArSoft 5.11) the standard
way to represent such an ID is evolving, and so we stick to a simpler
representation: a plain PDG ID. Keep in mind that using the full particle ID
from LArSoft might be more convenient, and that there must be a way to specify
that no ID information should be used.


### `CMakeLists.txt`                                                         ###

The provider interface does not require any linkage --- it has no code in.
The same would hold if it had some bit of inlined code.

The service provider implementations in this same directory do have code, and
this `CMakeLists.txt` makes sure a single library is created with all of them.
If needs arises, additional implementations may be linked into separate
libraries.
Dependencies include the LArSoft library with `recob::Shower` definition
(`lardata_RecoBase`). Some of the libraries we used from `larcore` are
header-only and they do not require linking to their library.

The resulting library name will be
`larexamples_Services_ShowerCalibrationGalore_Providers`.

Our examples follow a non-standard and not recommended practice: we install
`CMakeLists.txt` files and FHiCL files with `install_source()`. This is done
only because this is an example that should be distributed complete.
Normally, FHiCL files are installed in the correct location thanks to the
`install_fhicl()` directive, and `CMakeLists.txt` files are not installed at
all.


### `ShowerCalibrationGalore.h`                                              ###

The interface is coded into the abstract class `ShowerCalibrationGalore`.
We chose to have no actual code generated from its definition. This is both
common and usually recommended for abstract classes.

The class, in the customary `lar::example` namespace, implements a LArSoft
provider, that is required to be not copiable and not movable.
While in `AtomicNumber` example we did it explicitly, here we take a shortcut
by inheriting from a class that is not copiable nor movable, whose only purpose
is in fact that. It is `lar::UncopiableAndUnmovableClass`, provided by LArSoft
just for convenience. The derivation is not public since this class must not
contribute to the public interface.

In the class, a basic type for the correction is defined, `Correction_t`, that
holds a factor and its uncertainty. Also, a basic type for a PDG ID
specification is defined (just an alias to `int`).

The two interface methods are declared as pure virtual: all the implementations
will be forced to implement both of them. Their prescribed behaviour is
documented with Doxygen. The choice of `float` versus `double` is due to the
consideration that energy in LAr TPC is never measured with a resolution
requiring double precision representation. Other considerations might change
the decision: whenever the correction is used in double precision math, a
conversion will be performed and precision may be lost.
We add also a function describing the status of the correction object. The
content of the message is left to the implementer, but an implementation must
be provided.

An additional bonus is an operator able to insert a `Correction_t` factor
into an output stream. The `operator<<` is a template, so that it's not
necessary to know which kind of stream to use, as long as it supports the
needed operations (that here are `operator<< (Stream&, float)` and
`operator<< (Stream&, const char*)`. In this way, the operator can work with
both C++ streams (`std::cout`) and message facility objects (`mf::LogVerbatim`);
the "universal reference" `Stream&&` guarantees that temporary streams can also
be used.

Only a header file is needed.


### `ShowerCalibrationGaloreScale.h`                                         ###

The first, and simplest, implementation of the provider interface is 
`ShowerCalibrationGaloreScale`.
Its features are described in the class documentation in the usual Doxygen
format, including a more verbose description of the configuration parameters.

Our implementation includes the uniform correction factor as a data member,
and the interface will return its value for any energy and particle.

The class implements `ShowerCalibrationGalore` interface and therefore inherits
by it. It also inherits unmovability and uncopiability, LArSoft requirements for
service providers. The implementation details are quite similar to the ones in
`AtomicNumber` provider example. Among them, the Doxygen-based documentation
and the configuration validation structures.
An explicit choice was not to provide a default value for the correction, to
force users to actively choose one, even a neutral one like 1 +/- 0.

Note the use of the keyword `override` for all methods that we want to override
from the base class (in this case, we need to override all of them because they
were all pure virtual).

### `ShowerCalibrationGaloreScale.cxx`                                       ###

The implementation file for the provider is completely superfluous here.
We have placed a dummy one that just includes its header.
This makes sure that when we compile the providers library, C++ syntax of the
provider header is checked.

### `ShowerCalibrationGaloreScaleTestHelpers.h`                              ###

To support easy initialisation of this provider in a LArSoft test, we write
a couple of classes in this header. The header can be included by the tests
which need this kind of initialisation.
The use of these classes is described in the [Tests] section of this document.
In fact, we recommend to read that description before descending into the
following details of how to make that one work.

The header defines a class that have only inlined methods, and one that
is a template. Therefore it lacks an implementation file.

The first class we implemented is `testing::ProviderSetupClass`. This is a
templated class that is used by `setupProvider()` function to construct a
working instance of the specified provider. For this to work, a proper version
of this class must be implemented for our provider class,
`ShowerCalibrationGaloreScale`.
The type of construction we need, that is just to create a `new` provider object
using a FHiCL parameter set, is so simple that it is actually the one provided
by the default implementation of `ProviderSetupClass`. We acknowledge this by
explicitly instantiating the templated class for our provider. While this is
not required, it clearly states that we believe the standard setup is enough.

Descending in details, the `ProviderSetupClass` class is expected to have a
static member `setup()` that takes some arguments and returns a provider,
constructed and set up. The default `ProviderSetupClass` implementation, that we
are here accepting, takes all the arguments given to `setup()` and it forwards
them to the provider constructor. If `ShowerCalibrationGalore` required a
multi-step setup, this default implementation would not do, and we should
provide one that performs those steps (for example, a `setup()` function taking
as arguments a FHiCL parameter set and a geometry provider, which would first
create a new provider with the parameter set, then call provider's `Setup()`
method with the geometry provider, and then return it).

The second setup class is called `SimpleEnvironmentSetupClass`.
This is a one-shot setup class that assumes to be able to retrieve the
information it needs from an "environment", represented by a template class
`TestEnv`. This class is also expected to have a static `setup()` method taking
as argument the environment itself.
This is especially useful for tests that do not care about our provider, but
need it to be available because something depends on it. This function should
have a good chance to satisfy that dependency.

This class is a partial specialisation of the general one. The general one has
two arguments, one being the provider class, the other being the test
environment. We don't specify which test environment we want to support, and
we are happy as long as it can fulfil our needs, that are documented in the
description as "expose an interface equivalent to `testing::TesterEnvironment`.
The structure of this specialisation is simple: spell out the provider class,
and define a single static `setup()` function with a well defined prototype.
The content is also misleadingly simple: just a call to a function also provided
by LArSoft. This is one possible "standard" implementation. This function is
designed to work with a `testing::TesterEnvironment` and to set up the provider
from a certain available service parameter set (we chose
`"ShowerCalibrationGaloreScaleService"`, that will translate in the environment
looking for `"services.ShowerCalibrationGaloreScaleService"` parameter set in
the configuration: that is, the exact same key used in a valid _art_
configuration). The provider will be of type
`lar::example::ShowerCalibrationGaloreScale` (no surprise here), and it will
also fill in for `lar::example::ShowerCalibrationGalore` (that is, the interface
it implements).
Again, this is good enough for our provider. In the example above of a provider
requiring a multi-step setup and a geometry provider, this `setup()` method
might look like the following: first create the provider, possibly again by
`SimpleEnvironmentStandardSetupByName()`; then, ask the environment to yield
a pointer to the geometry provider, and set up the newly created provider with
it; and finally, return the pointer to the provider.
Note that the `setup()` in this class returns a bare pointer, while `setup()` in
the first one returned a `unique_ptr`. This is because the latter expects the
caller to manage the provider lifetime, while the former assumes that the test
environment is taking that task. A bare pointer represents non-ownership: this
is a fairly common assumption in modern C++, although it should be always
checked. The mysterious `SimpleEnvironmentStandardSetupByName()` function is
in its implementation explicitly telling the test environment object to manage
the provider.

Once we have understood how this works, we add an example of how to make use of
it in the Doxygen documentation.



### `ShowerCalibrationGaloreFromPID.h`                                       ###

This service provider is designed to read the corrections from a ROOT file, as
graphs (TGraphErrors). A specific correction is returned for one of a selected
number of particle types, or a generic one is returned if the particle is not
supported or if no particle ID was supplied.
The correction function is interpolated from the points read from file.
Uncertainty is also interpolated.

The additional complexity of this implementation is due only to the greater
complexity of the chosen task. The class inherits from the interface it
implements, `ShowerCalibrationGalore`, just as `ShowerCalibrationGaloreScale`
does, and it likewise has description in Doxygen format, configuration
validation, and redefinition of the essential interface functions.
The public interface of the class is slightly extended respect to the original
interface, by exposing a method to reload all the calibrations from a file,
and to print a report into any output stream. The latter is purely cosmetic
(note that it could not live in the abstract interface because C++ does not
support virtual templated methods).

The addition of `ShowerCalibrationGaloreFromPID::readCalibration()` is not
cosmetic, instead. We consider here that a _art_ service might decide to use
a different calibration depending on the run period the data being processed
belongs to, or on other additional information depending on the event, that
the provider does not know. In this case, the service provider will use this
method to communicate where to read the new calibration from.
We make the explicit choice here that it is responsibility of the framework side
to take care of having updated corrections.
Note that in _art_ context, the _art_ service interface is the only class that
can use the part of the provider implementation that is not in interface of
`ShowerCalibrationGalore`: neither modules or other services will be able to.
In a test context, the test is responsible of creating the provider and,
depending on the test, might choose to explicitly create this implementation and
gain access to its full interface.

Implementation choices include the definition of a private class to contain the
information of energy correction of a single particle type, with a range of
known validity derived from the input data; the use of a ROOT interpolation
class to provide a continuous representation of the correction out of the
discrete one supplied via graphs; some additional functions to find ROOT objects
inside a ROOT file; and little more.
The path from user's call to the returned number is as follow: first, the
calibration object (a `CalibrationInfo_t`) is chosen based on a hard-coded list
of possibilities. That object is then asked to provide the required information,
either just the correction faction or also its uncertainty. These are obtained
from interpolation objects that where initialised on construction from the data
in the input graphs. A slightly special logic is used for corrections with a
single point, that are considered uniform. In that case, the interpolation
object is initialised as a linear interpolator, supplying an additional fake
point to allows the actual interpolation.

Another important choice is to look for the calibration file in a location
provided in the environment variable `FW_SEARCH_PATH` (that is, needless to say,
standard in _art_). We rely on `cetlib` facilities to perform the search for us.

### `ShowerCalibrationGaloreFromPID.cxx`                                     ###

The implementation file here contains most of the actual code, except for short
methods that are better left inlined and for template method implementations.

### `ShowerCalibrationGaloreFromPIDTestHelpers.h`                            ###

This header mirrors exactly [`ShowerCalibrationGaloreScaleTestHelpers.h`].


### `showercalibrationgalore_standard.fcl`                                   ### 

We provide configuration examples for both the service providers.
We choose to put them in the same FHiCL file, although it's perfectly acceptable
and sometimes just better to spread them in different files.

Beside the usual documentation already described in the `AtomicNumber` example,
we have two different configurations.
They specify which provider they are for in a indirect way: they have a
`service_provider` key set to the _art_ _service_ name (as opposed to the
provider name). This is enough to have _art_ use them out of the box (as the
documenting header explicitly shows), and it does not bother the FHiCL
validation of the provider because `service_provider` is explicitly mentioned as
an exception.

The configuration for `ShowerCalibrationGaloreScale` provides in fact a valid
configuration, with a neutral calibration factor 1 +/- 0 that does not hurt.
This makes it the ideal configuration for experiments who don't want to bother
with shower energy calibration (yet).

The configuration for `ShowerCalibrationGaloreFromPID` does _not_ provide a
valid configuration out of the box. The calibration file is a required parameter,
and its content is something that is not possible to make usable for a "generic"
experiment. An option might be to create a calibration file with all neutral
corrections, to be stored it in `larsoft_data` repository. But given that it
would be exactly as using the other provider, it does not gain much.
We still provide a default configuration, that may guide the user to make its
own. The parameters that are required but we don't have a good value for are set
to the special FHiCL value `@nil`. Users can override it with something like
    
    services.ShowerCalibrationGaloreService.CalibrationFile: "actual/path.root"
    
or otherwise. If the `@nil` value reaches the FHiCL parser, the latter will
throw an exception and we'll know we have forgotten something.


## The service: `ShowerCalibrationGaloreService`  ##############################

The _art_ service interface is the object _art_ modules will look for.
The module will contain:
    
    ShowerCalibrationGalore const* showerCal
      = art::ServiceHandle<ShowerCalibrationGaloreService>()->provider();
    
or, completely equivalent:
    
    auto const* showerCal = lar::providerFrom<ShowerCalibrationGaloreService>();
    
The class `lar::example::ShowerCalibrationGaloreService` is just an interface,
implemented by a different class that will provide its own provider. What would
be for simple services to just ask for its provider:
~~~~
,---------------------.
| AtomicNumberService |
`---------------------'
           |
           |
           v
    ,--------------.
    | AtomicNumber |
    `--------------'
~~~~
becomes doubly indirect:
~~~~
,-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~.       ,-------------------------------------.
| ShowerCalibrationGaloreService |------>| ShowerCalibrationGaloreScaleService |
`-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~' (art) `-------------------------------------'
                                                           |
                                                           | (data member)
                                                           v
   ,-~-~-~-~-~-~-~-~-~-~-~-~-. (provider()) ,------------------------------.
   | ShowerCalibrationGalore |<-------------| ShowerCalibrationGaloreScale |
   `-~-~-~-~-~-~-~-~-~-~-~-~-'              `------------------------------'
~~~~
With the simple service, the caller knows the name of the interface, and
receives a pointer to a provider interface. For both the service and the
provider, the interface is implemented in the same class and the distinction
between interface and implementation is only formal.
In `ShowerCalibrationGalore` example, just as well, the caller knows the name of
the interface, and receives a pointer to a provider interface. But in this
case both service and provider interface do not come with an embedded
implementation. When the _art_ framework is requested for the service via
interface, it loads a concrete implementation. When the concrete implementation
is requested the provider, it returns the provider implementation it has
hard-coded inside. The provider implementation is returned converted to a
pointer to the provider interface, that is what the caller asked for.

Here we have to implement the service classes, one for the interface
(`ShowerCalibrationGaloreService`) and one for each of the implementations.
Each of them ends up being very similar to `AtomicNumberService`.
One major difference is the name of the preprocessor macros needed by _art_.


### `ShowerCalibrationGaloreService.h`                                       ###

The _art_ service interface is an abstract class.
It has a very simple task, and a very simple interface.
In fact, the only method in the interface is `provider()`, that is supposed
to return a pointer to the service provider as a provider interface
`ShowerCalibrationGalore`. In addition, we need to define the usual
`provider_type` data type that represents the provider interface.
No constructor is needed, because the class is abstract, but a virtual
destructor is needed.
This is so much boilerplate, that LArSoft provides a template for it.
It is called `lar::ServiceProviderInterfaceWrapper` because it wraps a service
provider interface, and takes the provider interface as template parameter.
There is a catch, though: the `provider()` method is not virtual. It returns
the result of another method: this other method, `do_provider()`, is a protected
pure virtual method. The derived classes will overload and implement this one to
return the pointer provider. One effect of this is to deprive the
implementations of the possibility to have `provider()` return anything else
than the provider interface. Using this template guarantees that the
requirements of `lar::providerFrom()` are satisfied, but you can code your class
from scratch as long as the same requirements are satisfied (see the
documentation of that function).
There is actually no class named `ShowerCalibrationGaloreService`: that is an
alias for the wrapper. This might create problems in some corner cases (for
example, you would not be able to forward-declare
`ShowerCalibrationGaloreService`) but it works for the standard case.
The alternative would be to have `ShowerCalibrationGaloreService` as class
publicly inheriting from the wrapper.

One last thing is needed: the macro `DECLARE_ART_SERVICE_INTERFACE`, similar
to `DECLARE_ART_SERVICE` but specific for a service _interface_, needs to be
invoked to inform _art_ about this interface.

The header ends up being fairly short thanks to the boilerplate (it would not be
much longer otherwise). More room for documentation! Unfortunately the `using`
trick places the section about this service interface in the non-intuitive place
of the `typedef`s.
Conversely, not much documentation is needed here, since the real deal is in
the provider interface.

Being an abstract interface, no implementation file is required (and no macro
for the "definition" is needed either).


### `ShowerCalibrationGaloreScaleService.h`                                  ###

The class `ShowerCalibrationGaloreScaleService` implements a "concrete"
`ShowerCalibrationGaloreService` _art_ service. For normal use (e.g. in _art_),
the service will appear only in the configuration of the job, as the
`service_provider` of `ShowerCalibrationGaloreService` service.

Its functionality is to manage and keep updated a service provider, in this case
an instance of `ShowerCalibrationGaloreScale`. The service needs to derive from
the abstract interface `ShowerCalibrationGaloreScale`, and it needs to implement
one or more constructors and the pure virtual `do_provider()` method that was
left dangling in the interface.
The constructor required by _art_ can have one of two signatures, taking as
arguments a FHiCL parameter set and a activity registry, or a FHiCL
configuration table and an activity registry. The first argument will supply
configuration to the service, that is going to forward it to the provider,
without and with parameter validation respectively for the two signatures.
The second argument is common to the two signatures and it is a facility _art_
provides for the service to be "informed" when the framework changes status,
as for example when it starts processing an event, or starts a new run, or
closes an input file or is ending the job. The service is supposed to register
a "callback" function that will be executed when the bound condition is met.
For example, a service might decide to print a summary at the end of the job,
by registering a `PrintSummary()` method bound to the `endJob` condition.
In general, in the factorisation model the service would inform the provider
of the changed condition and trigger any change needed. This example does not
utilises this feature.

We provide only the configuration validating constructor --- _art_ will take
care of the rest.
We have decided to create the service provider immediately on construction.
The construction of the service provider is simple: just pass the configuration
to the provider constructor, and we are done. The `do_provider()` implementation
is as trivial as to return the data member which contains the provider instance.
We define some optional types to give some information about which interface
we are implementing (`service_interface_type`) and which is the actual provider
we manage (`concrete_provider_type`). The rest is similar to
`AtomicNumberService` example (note that the `provider_type` type is inherited).

Note that if the construction of the provider requires a lot of resources and
you expect that it might be unused even when configured (if for example it
provides a service useful only in one event out of one hundred, a job with
twenty events is likely not to invoke it), then a different, "lazy" construction
pattern might be preferred. In that pattern, the constructor just saves a copy
of the configuration, but it is `do_provider()` that constructs the provider
if it's not yet available (that is, the first time `provider()` is called).
This pattern may clash with configuration validation. Another pattern is to
indeed construct the provider on service construction, with a constructor that
does not perform the full initialisation. That initialisation would be then
triggered by `do_provider()`. This pattern allows a correct configuration
validation but it requires explicit support by the provider.

All this code is very patterned, and LArSoft does provide a template to save the
typing, that will be used by the other service implementation,
`ShowerCalibrationGaloreFromPIDService`. The provider must expose a certain
interface, which `ShowerCalibrationGaloreScale` also does. Here we haven't used
it just for illustration purposes.

Note the preprocessor macro `DECLARE_ART_SERVICE_INTERFACE_IMPL` specific for
a _art_ service implementing an interface.

Finally, the documentation mainly sends the reader to the description of the
service provider, but it also reminds how write the configuration so that _art_
uses this service implementation. It also reminds of the fact that this code
should be basically invisible: access to the functionality of the provider is
obtained through its interface `ShowerCalibrationGalore`, access to the provider
is obtained through the _art_ service interface `ShowerCalibrationGaloreService`
and nowhere in the code `ShowerCalibrationGaloreScale` or its related service
`ShowerCalibrationGaloreService` should appear. Not even in the link list of
user code! The framework is expected to load and dynamically link the correct
library (that might be the one containing `ShowerCalibrationGaloreScaleService`
or another one, depending on the configuration) as needed.


### `ShowerCalibrationGaloreScaleService_service.cc`                         ###

The implementation of the service is again almost unnecessary, as it was in
`AtomicNumberService` example. Here a preprocessor macro specific for service
interface implementation must be used, `DEFINE_ART_SERVICE_INTERFACE_IMPL`.


### `ShowerCalibrationGaloreFromPIDService.h`                                ###

The service implementation `ShowerCalibrationGaloreFromPIDService` manages an
instance of the service provider `ShowerCalibrationGaloreFromPID`.
Beside the difference in which service provider is being managed, the
functionality of this service is exactly the same as the one of the other
service implementation, `ShowerCalibrationGaloreScaleService`.

Here we take advantage of the template for the simple implementation of service
interfaces `lar::ServiceProviderImplementationWrapper`. As the name suggests,
this template wraps a service provider implementation. The two template
arguments are the provider implementation and the _art_ service interface.
Note that these two classes don't know each other, although they both know the
provider interface `ShowerCalibrationGalore`.
The requirements to use this wrapper are documented with the wrapper itself, and
they can be summarised in: the provider must have a configuration object called
`Config` and a constructor that takes one of them as only argument.
In addition, this template can't be used as is, when callback methods need to be
registered.


### `ShowerCalibrationGaloreFromPIDService_service.cc`                       ###

This implementation is completely equivalent to the one in
[`ShowerCalibrationGaloreScaleService_service.cc`].


### `CMakeLists.txt`                                                         ###

Each service must live in its own library, and that library must have a specific
name, so that _art_ can connect it to the name of the service it contains and
load it. This is a requirement that does not hold for the service provider
library or libraries, but only for the _art_ services.

Here we let `cmake` macro `art_make()` to take care of all the rules and
requirements. We only specify that all our services must be linked with the same
set of libraries. Noteworthy are the library with the provider implementations
and `lardata_RecoBase` for `recob::Shower` (that might be in this case actually
optional since the services don't make any use of that object).
Note that although we did use some header from `larcore`, those headers have no
implementation file and are not connected to any library, as can be read in the
documentation of those very headers.

The `art_make()` macro will ensure that one library is created for each of the
source files `*_service.cc` present in the directory. The result can be seen in
the `${MRB_BUILDDIR}/larexamples/lib` directory.

If we had the need of different library sets, we would have used
`simple_plugin()` macro instead of `art_make()`.



## Tests  ######################################################################

As in `AtomicNumber` example, we have stand-alone tests for each of the service
providers, and a test under _art_.
Differently from the `AtomicNumber` example, we are using here a part of LArSoft
test facility expressly designed for tests that use services.
We have described in [`ShowerCalibrationGaloreScaleTestHelpers.h`] file the
additional work that it takes for a service provider to support these features.
Here we see how to use them.

The following tests are implemented:

* `CreateTestShowerCalibrationFromPID_test`: creates a file that can be used as
  calibration data by `ShowerCalibrationGaloreFromPID` provider
* `ShowerCalibrationGaloreScale_test`: unit test for
  `ShowerCalibrationGaloreScale` service provider
* `ShowerCalibrationGaloreFromPID_test`: unit test for
  `ShowerCalibrationGaloreFromPID` service provider
* `test_shower_calibration_scale.fcl` for a _art_ test using
  `ShowerCalibrationGaloreScaleService`
* `test_shower_calibration_from_pid.fcl` for a _art_ test using
  `ShowerCalibrationGaloreFromPIDService`

As a reminder, when `mrb test` is used, all the tests are run automatically and
a global output log is also available, typically in
`${MRB_BUILDDIR}/Testing/Temporary/LastTest.log`.


### `CMakeLists.txt`                                                         ###

Tests are as always declared to `cmake` via the `cet_test()` macro.

For the _art_ service test, we have a test module that needs to be built.
We use the `simple_plugin()` macro specifying the name of the module and the
link list. The link list does _not_ include any library related to
`ShowerCalibrationGalore`. The test module uses the abstract service interface,
and it will be _art_ to decide which libraries to link at run time.

There are two tests driven by _art_, that exercise the two service providers.
The other tests have their own executable file.

As mentioned while describing the other `CMakeLists.txt`, we perform here a more
complete installation than it would usually be done for non-example code.


### `CreateTestShowerCalibrationFromPID_test.cc`                             ###

This is not really a test... maybe it's a metatest. It creates a file that is
suitable to be used as `ShowerCalibrationGaloreFromPID` calibration data.
The physics content is complete naught, but the form is correct.
One of its purposes is to show an example of calibration input file for the
provider. Another is to allow the creation of calibration input on the spot,
so that a test can use it immediately.

The actual creation of the calibration file is delegated to a separate
compilation unit, `CreateTestShowerCalibrationFromPID.h` and its implementation.
This exposes a function to create an arbitrary calibration file.
The unit test `ShowerCalibrationGaloreFromPID_test` will use that function to
create the file to configure its provider with.

This "test" was also used to fill the `examples/CalibrationExample.root` file
with shower calibration data. That file is stored in `larsoft_data` package,
which is automatically configured together with LArSoft, and it is the file
that the _art_-based test uses to configure
`ShowerCalibrationGaloreFromPIDService`.


### `ShowerCalibrationGaloreTests.h`                                         ###

This header contains a "test" function that prints the correction factor for a
synthetic shower within a range of energies, and for a selection of particle
IDs. It is not a real run-time test in the sense that it does not validate the
correction values and can't detect failures. It will still somehow exercise the
service.

The function only uses the provider interface, `ShowerCalibrationGalore`, and
can therefore be run with any provider implementation.

The output function uses a template parameter for the output stream.
As a consequence, the whole code is in a header and no implementation file is
required.


### `ShowerCalibrationGaloreScale_test.cc`                                   ###

This test exercises the provider implementation `ShowerCalibrationGaloreScale`.
The test uses a known configuration and verifies that the correction factor for
a synthetic shower of a certain energy matches the expected one.
It also prints a correction table on the screen via the utility in
`ShowerCalibrationGaloreTests.h`, but it does not perform any check on the
output.

#### The test environment

This test uses the same facility as `AtomicNumber_test` to establish a test
"environment", that is a class to refer to for global status like configuration
and... service providers.
The `TesterEnvironment` class does not know anything of any service provider,
but it can figure out given the proper directions.
The example begins with the creation of a configuration object.
In this object, of type `testing::BasicEnvironmentConfiguration`, we insert a
default configuration for the service called `"ShowerCalibrationGaloreService"`.
The configuration is made of three lines:

1. specification of `factor` as a known value from a constant
2. specification of `error` as a known value from another constant
3. specification of a `service_provider` of type
   `"ShowerCalibrationGaloreScaleService"`

This is a complete _art_ configuration for `ShowerCalibrationGaloreService` that
uses `ShowerCalibrationGaloreScaleService` as implementation.

We create a test environment object by invoking the function
`testing::CreateTesterEnvironment()` with that configuration. Not much magic
involved here: a `TesterEnvironment` class is created, and the use of this
function just saves us typing.

Finally, we tell the test environment to set up the provider
`lar::example::ShowerCalibrationGaloreScale` and cross our finger hoping it does
know how to, since we don't want to. Here some magic does happen.
The tester environment looks for a template class `SimpleEnvironmentSetupClass`
specialised with `lar::example::ShowerCalibrationGaloreScale`, and executes the
static `setup()` method in there. That class and method is there only if the
author of the service provider put it there, which we in fact did (see
[`ShowerCalibrationGaloreScaleTestHelpers.h`]). Good for us here: we just have
to include that header.
If `ShowerCalibrationGaloreScale` depended on other services, we should have
them set up first, in one way or the other. One way is the same
`SimpleProviderSetup()` call for those other providers. For geometry service,
it's currently complicate enough that a specific tester environment should be
used (you can see the unit test of `detutil::DetectorPropertiesStandard` in
`lardata` repository).

A few lines later we access the provider with a call to test environment method
`Provider<lar::example::ShowerCalibrationGalore>()`, and a provider of type
`lar::example::ShowerCalibrationGalore` is ready to be used, just as when we ask
_art_ in a module: `lar::providerFrom<lar::example::ShowerCalibrationGalore>()`.

The rest of the test is simple. We call the function that prints the correction
table passing a pointer to the service provider, a energy range and a list of
particle IDs.
Then we ask to print a report from the provider on the screen.
Finally, the real, small test: we create a synthetic shower object with
`lar::example::tests::MakeShower()`, specifying only its energy. The function
will assign some dummy value to all the other quantities the shower requires.
We ask for the correction factor, and we verify that it's the same we configured
in some lines above.


### `ShowerCalibrationGaloreFromPID_test.cc`                                 ###

The test for `ShowerCalibrationGaloreFromPID` provider mirrors the one for
`ShowerCalibrationGaloreScale`. They differ in some details, though.

This service provider requires a calibration file as input. Here we don't want
to guess, so we invoke a function from the test library that creates one for
us with the name we want.

Then we proceed to configure the service and the environment by the same means
we used for the other provider. The configuration will be still stored as
`"ShowerCalibrationGaloreService"`, because that's what would happen in a _art_
context, and we invoke the "simple setup" of the provider implementation we are 
testing. The access to the provider uses exactly the same line of code as the
other test.
The difference is that here it's not as simple to guess which correction to
expect, and this test turns out to be not really much of a test at all.
We don't verify that the correction values are as expected, because we don't
have an expectation. In a real test, the expectation might be provided by the
function or set of functions that produces the calibration file.

A detail here is that we used message facility instead of C++ output streams to
print the output. Just to show that we can.


### `ShowerCalibrationGaloreTest_module.cc`                                  ###

Tests of _art_ services use a simple module. There is just one test module,
that we'll run with two different configurations, one for each service
implementation.

The test module is not much. Given its generality, it can't guess which
correction values are expected, so it ends up being a glorified version of 
`ShowerCalibrationGaloreFromPID_test.cc`. It sports configuration validation,
just for habit, and will print the correction table thanks to the usual function
from `ShowerCalibrationGaloreTests.h`.

It is not a very good test, although it's good enough to verify that the service
correctly loads and is invoked. Yet it is a good example of a module using the
service interface. And once more, note that no mention is made of any specific
service implementation throughout the module, nor in the module configuration.
The information about the service implementation starts and ends in the
configuration of the service itself in the FHiCL configuration file.


## Questions?  #################################################################

If you have any question about the example, please contact its author.
This section will be populated with questions and their answers.

