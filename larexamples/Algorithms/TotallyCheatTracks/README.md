#  An algorithm producing a new data product: `TotallyCheatTrackingAlg`  {#TotallyCheatTracks_README}

| Example name:   | TotallyCheatTracks                    |
| --------------- | ------------------------------------- |
| Type:           | LArSoft algorithm with module         |
| Author:         | Gianluca Petrillo (petrillo@fnal.gov) |
| Created on:     | December 26, 2017                     |
| Version:        | 1.0                                   |


This example shows an algorithm whose output is stored in a new data product.

Features of the algorithm and module:

* most features are in common with the example `RemoveIsolatedSpacePoints`
* an algorithm fills a new class, which a _art_ module stores as data product
* associations to input objects are also produced

Missing features that you need to go elsewhere if you need to implement:

* complete unit test (some test is still present)
* `lar_ci` integration test

Technical choices:

* employs the module/algorithm factorisation model
* uses validation of the configuration
* it is fully documented via Doxygen interpreted comments


### How to use this document                                                 ###

This document is pretty long. You are encouraged to read it all, but you might
understandably opt out of that.
To make it even longer, we assume that you are aware of the existence of the
_art_ introductory documentation (in particular, the _art_ workbook). Having
read it will imbue the present text with a lot more sense. And if you have no
idea about what _art_ is, or what a _art_ module is, you definitely need to go
there first. Also, most of the information that is already present in LArSoft
example `RemoveIsolatedSpacePoints` is not repeated in its entirety here.

This document is organised in sections. We suggest that you read the section
above to know what this example is about.
Then, you can dig into the code and come back to the pertinent section when you
have questions.

For every questions, answered or not here, you are strongly encouraged to
contact the example's author (contact information is at the top of this file).

And, if you want to have a bit more printer friendly format, know this text file
is written in markdown format and you can convert it to something else with:
    
    pandoc -s -S --toc -o README.html README.md
    pandoc -s -S --toc -o README.pdf README.md
    
et cetera.



## The creation of data products  ##############################################

A "data product" is a class that can be saved in a _art_ ROOT output file.
In order for any class to become a data product, it needs to satisfy a few
requirements. The hardest part about creating a good data product is its design:
that class will be saved in the output of your experiment forever, and once it's
saved it can be hard to change it without breaking the old data and code.

This is not a guide to data product design, and the choices made in this example
may be not motivated in detail. More information is available in [art wiki] and
some LArSoft code conventions are described in the [LArSoft wiki].



## Files  ######################################################################

The core example is split between two directories:

~~~~
larexamples/Algoritmhs/TotallyCheatTracks/
|-- README.md                                                        # this file
|-- CheatTrackData                         ## contains the data product class ##
|   |-- CheatTrack.h                                # data product class header
|   |-- classes.h              # file for ROOT to generate class dictionary (I)
|   `-- classes_def.xml       # file for ROOT to generate class dictionary (II)
|-- TotallyCheatTrackingAlg.h                                 # algorithm header
|-- TotallyCheatTrackingAlg.cxx                       # algorithm implementation
|-- cheat_tracks.fcl                          # algorithm standard configuration
|-- TotallyCheatTracker_module.cc         # art module writing the data products
`-- totallycheattracking.fcl                         # example job configuration
~~~~

Each (sub)directory also has its own `CMakeLists.txt` file.

Tests are in `test/Algoritmhs/TotallyCheatTracks` directory.


### Directory structure                                                      ###

The directory structure in this example is comprised of a main directory
(`TotallyChearTracks`), and a subdirectory containing only the data product
(`CheatTrackData`). This separation is not required but it is recommended,
because it guarantees that the data product libraries have no unexpected
dependencies. This is more important for data products because its code and
library may end up in other code which does not know of LArSoft or of _art_
(e.g. _gallery_).

While the factorisation model is still applied, respect to the example
`RemoveIsolatedSpacePoints`, here the algorithm and module share the same
directory. This choice should be avoided in normal code, but it is used here for
simplicity, because factorisation is not the focus of this example.


## The data product class ######################################################

The data product we need represents a reconstructed track, and we choose to
characterise it with:
* a trajectory (position and momentum);
* a particle identification code; we use PDG codes.

The most important part of the data product class is to explain, in the
documentation, what it is meant to represent. Failure to do so has caused and
still causes countless troubles in LArSoft.

The design that is recommended by _art_ and LArSoft is of a class whose content
_can not be modified after construction_. Therefore, the constructor arguments
must fully define the final content of the object. While this may be tedious on
the caller side, it is considered good practice.
More important, the data product should expose a stable access interface, and
hide the internal representation of the data. The implementation in this example
goes almost all the way to it:
* data members are private
* access to the data is via member functions

We use a standard LArSoft data class for the interface exposing the trajectory
concept, `recob::Trajectory`. In this first version, the interface returns that
object directly from an internal data member, which is acceptable although it
might make a future change of internal representation cumbersome. Also it is
important to make sure that the concept represented by `recob::Trajectory` fits
well with the concept in our class: each evolution of `recob::Trajectory` will
be reflected in our object.

We choose to give a name to the particle ID type (`PDGID_t`). In the future,
that might become a more complex object, but the change might be absorbed by the
type name and by a proper design of the new class interface.

We write some method to print the content of the object to screen. This is very
useful when debugging and for messages on screen (e.g. errors). We chose to go
with a more powerful method `dump()` able to output on any stream (including the
usual STL output streams like `std::cout`, but also the message facility loggers
like `mf::LogInfo()` and `LOG_DEBUG()`), with control on the amount of detail
printed and on the indentation of the output, and to have an inflexible output
in the traditional `operator<<`, which must be implemented as a free function
_in the same namespace_ where the data product is defined.

We also need to provide a default constructor, because ROOT requires it to read
objects of this type from ROOT trees. We let the compiler write the default
constructor, but we specify that the default particle ID is an invalid one,
`InvalidParticleID` (specifically, value is `0`).
This constructor needs to be public but it will be of little use to normal
users, since the object can't be changed after construction.

We have added a name for the invalid particle ID and a method to check its
validity in a easy way. Experience shows that checks with the plain value get
confusing and `if (!part.hasParticleId())` is less likely to be misinterpreted
than `if (part.particleId() == 0)`.

Normally, we should also provide a unit test. Unit tests for these simple data
products are also simple, and may consist of constructing an object with the
main constructor and checking that all the accessors give the expected result,
and doing the same for the default constructor.

The style convention is drinking camel case for member functions (e.g.
`particleId()`, with all words separated not by an underscore, but by the
capitalised first letter of each word, except the first one),
vigilant-camel-case for data types (e.g. `PDGID_t`, the first letter also
capital), and data member names prepended by `f` (e.g. `fTraj`).

The class is defined in a header file (`CheatTrack.h`). All member functions are
very simple and are (and should) be implemented inline, that is in the header.
The only exception is the `dump()` method, which is not very simple but is a
template and therefore also needs to be implemented in the header file.
This means that there is no implementation file (no `CheatTrack.cxx`), and also
that there will be no library `CheatTrackData` or similar (it would have been
`liblarexamples_Algoritmhs_TotallyCheatTracks_CheatTrackData.so` on Linux).
So we don't need to add to the link lists that use `lar::example::CheatTrack` a `larexamples_Algoritmhs_TotallyCheatTracks_CheatTrackData` line in the link
list: including the header in the C++ source files will be enough.


## The algorithm and module ####################################################

This section is succinct and does not describe the technical choices in the
module and algorithm code. Nothing is new here with respect to the example
`RemoveIsolatedSpacePoints`.

The (preposterous) algorithm `TotallyCheatTrackingAlg` accepts a single
simulated particle (`simb::MCParticle`, like the ones produced by `LArG4` module
and in the `simb::MCTruth` objects produced by event generators), and
"reconstructs" it into a new data product `lar::example::CheatTrack`.
The module drives this algorithm and performs a minimal selection on the input
collection of simulated particles: only particles with a minimum energy and
path length will be processed. The module also saves an association between
the input particle and the reconstructed `CheatTrack`.


### Algorithm design                                                         ###

The design of the algorithm here is not explained in detail, and we refer to the
usual example `RemoveIsolatedSpacePoints` for most of the explanations.
A few notes follow about peculiarities of this design which are not present in
that example.

The algorithm presents the complete interface as recommended for a stateless
algorithm: a constructor with (validated) configuration, a setup phase, and a
compact execution method which merges input, processing and output. Due to the
simplicity of the algorithm, though, the configuration is empty and the setup is
no operation. While these parts are unnecessary and not mandated, it's not a bad
idea to leave them anyway for future extensions.

The most important difference in the design is that this algorithm processes a
single input object rather than a collection of them. It is up to the caller to
make sure that all the objects which need to be converted are processed.
This has some consequences:
1. the caller module gets to put together the data product
2. input/output object associations must also be managed by the caller module
3. the algorithm is independent of the way the input is stored

The last point is particularly relevant for this case in LArSoft: the module
`LArG4` produces a collection of `simb::MCParticle`, and the algorithm could
have easily been written to accept a `std::vector<simb::MCParticle>` as input.
Yet, `simb::MCParticle` objects are also stored as part of `simb::MCTruth`, and
that object does not offer them as a collection but only individually. A similar
situation may be found in other parts of the code, and it's often a good choice
to have the algorithm process the smallest unit possible (`simb::MCParticle` in
this case), or to accept as input a collection of pointers to the objects
(`std::vector<simb::MCParticle const*>`), so that the conversion from other
formats at least does not require to _duplicate_ the content of the input.


### Creation of the associations                                             ###

The module is in charge of creating the association between the input and output
data. This design choice is twofold. First, the algorithm is designed to deal
with the single output, as described in the previous section. Second, it allows
the algorithm to be more independent of framework concepts like the _art_
pointers required to create associations.

An association is a template data product provided by _art_, which connects
two elements from different data product collections, and with an optional
"payload" of data describing each connection. The `art::Assns` class is a
collection of such connections; in jargon, this collection may be referred to as
"association" or "associations". In our example, the connection is between
`simb::MCParticle` and `lar::example::CheatTrack`, with no payload, and this is
represented by the associations class:
    
    art::Assns<lar::example::CheatTrack, simb::MCParticle>
    
The first argument (here `lar::example::CheatTrack`) is sometimes called the
"left" type, and the second argument (`simb::MCParticle`) the "right" type.
The payload would be the third template argument; it is rarely used and its
default is `void`, which means no payload at all.
Associations are bidirectional, in that they store enough information to allow
retrieving which `simb::MCParticle` object(s) are associated to a
`lar::example::CheatTrack`, and which `lar::example::CheatTrack` object(s) are
associated to a `simb::MCParticle`. Also, objects on both sides can be shared by
different connections ("many-to-many" associations).

The basic way to add a connection to the associations object is via the method
`addSingle()`, which takes as arguments the _art_ pointer to the left object and
one to the connected right object (and the payload data to be copied, when it
applies). LArSoft used to provide some functions called `util::CreateAssns()`.
These functions have been deprecated because they are so hard to use correctly,
that they cause often unnecessary overhead and sometimes plain mistakes.

The example member function `lar::example::TotallyCheatTracker::produce()` shows
how to use `addSingle()` to add a connection. It is as simple as:
    
    trackToPart->addSingle(trackPtr, partPtr);
    
where the two arguments are _art_ pointers to the objects to be connected.
The complication is only how to create those two pointers. This depends on
whether the data product already exists in the event (like `simb::MCParticle`)
or if we are creating it now (like our output `lar::example::CheatTrack`).
_art_ provides a utility `art::PtrMaker` which almost uniforms the interface
to create the pointers. Once a `art::PtrMaker` object is created, the only
necessary information to create a pointer to an object is the index of that
object in the data product collection, whether the object is already there, or
it will put there at a later time. The method instantiates two of these objects:
    
    art::PtrMaker<simb::MCParticle> makePartPtr(event, particleHandle.id());
    art::PtrMaker<lar::example::CheatTrack> makeTrackPtr(event, *this);
    
The `art::PtrMaker` class accepts as template parameter the class the pointers
will be pointing to. Then:
* if the target data product already exists, `art::PtrMaker` must be constructed
  with the ID of that data product; the ID can be obtained from the data product
  handle (as in the example);
* if the target data product is going to be produced by the current module, the
  module itself needs to be passed to `art::PtrMaker` constructor (if the target
  data product also has a instance name, that name must be specified too as the
  third argument).

In the particle loop, the module keeps track of the index of the input particle.
This may be achieved by a range for loop and a separate index (`iParticle`),
which has potentially slightly better performance but is more error prone
(remember to always increase the index, and do that at the right time!).
An alternative is to go with a index-based for loop (as in the example); another
one is to have a iterator loop with an index... whatever it takes.
The track index is easier, because when we are adding the connection to a track,
that track has just been appended to the track collection, and therefore its
index is the size of that collection minus one.
Note that the whole thing may have been written in a more compact way as:
    
    trackToPart->addSingle
      (makeTrackPtr(tracks->size() - 1), makePartPtr(iParticle));
    
Also note that the creation of a _art_ pointer for an existing data product is
very simple also without `art::PtrMaker`: in this case it would have been:
    
    art::Ptr<simb::MCParticle> const partPtr { particleHandle, iParticle };
    
We use `art::PtrMaker` for uniformity (also, it's negligibly more performant).



### The ROOT dictionary                                                      ###

The ROOT "dictionary" is a library produced by ROOT that enables introspection
of the classes. Type introspection is a feature which allows to ask, at run
time, which methods and fields are present in the class, to execute the former
and ask the value of the latter. It is the key feature of the ROOT interpreter
and of many other ROOT facilities, including the input and output of almost any
C++ objects in a ROOT tree.

The build system is able to ask ROOT to create a dictionary when provided with
a few instructions. And ROOT (`rootcling`) can do that when provided _a lot_
more instructions.

#### `classes_def.xml`

The `classes_def.xml` contains the list of _all_ the classes for which a
dictionary is needed. It is quite tedious a list to compile, and on top of that
its format is XML (almost!).

The XML format is almost standard, except that the characters `<`, `>` and `&`
are allowed unescaped in the attribute values, which is self-evidently good
when one has to add a class like:
    
    std::pair<art::Ptr<lar::example::CheatTrack>, art::Ptr<simb::MCParticle>>
    
(as we do) without escaping all those angular parentheses.

The `classes_def.xml` file also takes note of the different _versions_ of a
class. The feature coping with the changes of data product classes is called
_schema evolution_. For example, if we run for a while with the current version
of `lar::example::CheatTrack`, and then we add a `fType` data member, the class
changes and when ROOT is given an old file (where `lar::example::CheatTrack`
has no `fType`) it needs to know that there is no `fType` in the data, and how
to fill in a default value. While a remarkable level of complication may become
involved, ROOT is able to correctly react to the simplest changes with no user
action. The important thing is to inform ROOT that a class _may_ change, by
explicitly assigning a `ClassVersion` number (the first version number is
strongly recommended to be `10`). The resulting magic line we write in
`classes_def.xml` is then:
    
    <class name="lar::example::CheatTrack" ClassVersion="10" />
    
(this is a "empty element" XML tag, `<TAG/>`).

*Note*: when the build system realises the class is changed, it will _update the
`classes_def.xml` file_, issue an error on screen and ask to rebuild again.
For example:
    
    INFO: adding version info for class 'lar::example::CheatTrack':<version ClassVersion="10" checksum="790383593"/>
    WARNING: classes_def.xml files have been updated: rebuild dictionaries.
    
Rebuilding is expected to succeed. Also make sure that you don't overwrite the
file after it has been changed by the build system (for example, if you have the
file open in an editor and you save it again without reloading it first) and
that you commit the updated `classes_def.xml` to your GIT repository.
You may notice that the empty element tag has been replaced with a complete tag:
    
    <class name="lar::example::CheatTrack" ClassVersion="10" >
     <version ClassVersion="10" checksum="790383593"/>
    </class>
    
(`<TAG> ... </TAG>`); the tag contains now a list of versions, and for each a
checksum extracted from the class at that version. The next time the class is
changed, the build system will again add a tag `<version ... />` to that list
and ask to build again.

The definition of which classes we need to appear in this file is the most
troublesome. The list starts simple: since we want the class
`lar::example::CheatTrack`, we add the corresponding line:
    
    <class name="lar::example::CheatTrack" ClassVersion="10" />
    
Then we need to specify all the classes _it_ needs. `lar::example::CheatTrack`
contains a `recob::Trajectory`, therefore that class needs to be known to ROOT.
Fortunately, `recob::Trajectory` is already a data product on its own merit, and
a dictionary for it is generated in `lardataobj`. But what we want to write in
the _art_ ROOT file is actually a `std::vector<lar::example::CheatTrack>`, and
as a consequence that class also needs its entry in `classes_def.xml`:
    
    <class name="std::vector<lar::example::CheatTrack>" />
    
(we don't expect the class `std::vector` to change, so we don't bother to
specify a version for it). Furthermore, unbeknownst to us, what actually _art_
writes is a wrapped version of that vector, and it's on us to ask ROOT to
generate a dictionary for it too:
    
    <class name="art::Wrapper<std::vector<lar::example::CheatTrack>>" />
    
It's also good practice to generate a dictionary for the class _art_ pointer,
`art::Ptr<lar::example::CheatTrack>`. This takes finally care of the data
product itself.

Things go wild with associations. The relevant object is
`art::Assns<lar::example::CheatTrack, simb::MCParticle>` (1 entry), and the data
product is wrapped in
`art::Wrapper<art::Assns<lar::example::CheatTrack, simb::MCParticle, void>>` (2
entries so far). But the `art::Assns` internally uses a `std::pair` of pointers:
`std::pair<art::Ptr<lar::example::CheatTrack>, art::Ptr<simb::MCParticle>>`
(3 entries so far). Ugh. And the story about the bidirectionality of
`art::Assns` also implies that dictionaries should be available when swapping
left and right class: `art::Assns<simb::MCParticle, lar::example::CheatTrack>`
and related classes (double everything: 6 entries!).
Now, did we say `art::Ptr`? well, those too need to have a dictionary.
We already requested one for `art::Ptr<lar::example::CheatTrack>` above, while
the one for `simb::MCParticle` is already provided in _nusimdata_.
And this concludes the list. Take a look at the `classes_def.xml` in
`lardataobj/RecoBase` for guaranteed headache.

#### `classes.h`

The C++ header `classes.h` is expected to include all the class and type
declarations that are needed by the classes we want the dictionary of.
The included headers need to be enough to declare all the classes involved in
`classes_def.xml`. That usually includes the class headers (here, one for
`lar::example::CheatTrack` and one for `simb::MCParticle`), plus `Assns.h` and
`Wrapper.h` from _art_ (`art::Ptr` is used by `art::Assns` and therefore
indirectly included already, but it would not hurt to include it explicitly).

#### `CMakeLists.txt`

The _art_ build system will create a ROOT dictionary automatically when
the macro `art_make()` is used in `CMakeLists.txt`, if the files `classes.h`
and `classes_def.xml` are present in the same directory. In our example, we
needed to tell it about two additional libraries needed for linking, since we
ask for the dictionary of associations to `simb::MCParticle` (from
`nusimdata_SimulationBase` library in _nusimdata_). We specify the extra library
after the `DICT_LIBRARIES` keyword of `art_make()`. It is also possible to
create only the dictionary by using `art_dictionary()`.


### Unit tests                                                               ###

A minimal unit test is provided to prove that the dictionary generation works
properly. The test, in `larexamples/test/Algorithms/TotallyCheatTracks`, is
shortly described here.

The test is an _art_ test using Boost unit test, so it is run with `lar_ut`.
The configuration, `test_totallycheattracker.fcl`, first creates some simulated
particles with a custom test module, then runs the tracker `TotallyCheatTracker`
and finally runs a custom dumper which prints the particles and its
associations. The output file can be used to verify that the data product was
written correctly. For example, we can have a run in gallery:
    
    setup larsoftobj v1_34_00 -q prof:e14
    root -l <<EOC
    gallery::Event event({ "TotallyCheatTracker_test.d/CheatTrkTest.root" });
    auto const& tracks = *(event.getValidHandle<std::vector<lar::example::CheatTrack>>("cheattrk"));
    for (auto const& track: tracks) std::cout << track << std::endl;
    EOC
    
will print something like:
    
    Successfully opened file TotallyCheatTracker_test.d/CheatTrkTest.root
    particle: mu+ (ID=-13); momentum: 2 GeV/c; trajectory with 5 points at ( 1 ; 2 ; 3 ) cm toward ( -1 ; 0 ; 0 ) with momentum 2 GeV/c
    ends at ( -3 ; 2 ; 3 ) cm toward ( -1 ; 0 ; 0 ) with momentum 2 GeV/c
    particle: pi+ (ID=211); momentum: 3 GeV/c; trajectory with 6 points at ( -3 ; 2 ; 3 ) cm toward ( 0 ; -1 ; 0 ) with momentum 3 GeV/c
    ends at ( -3 ; -3 ; 3 ) cm toward ( 0 ; -1 ; 0 ) with momentum 3 GeV/c
    particle: pi0 (ID=111); momentum: 4 GeV/c; trajectory with 7 points at ( -3 ; -3 ; 3 ) cm toward ( 0 ; 0 ; -1 ) with momentum 4 GeV/c
    ends at ( -3 ; -3 ; -3 ) cm toward ( 0 ; 0 ; -1 ) with momentum 4 GeV/c
    particle: pi- (ID=-211); momentum: 3 GeV/c; trajectory with 8 points at ( -3 ; -3 ; -3 ) cm toward ( 1 ; 0 ; 0 ) with momentum 3 GeV/c
    ends at ( 4 ; -3 ; -3 ) cm toward ( 1 ; 0 ; 0 ) with momentum 3 GeV/c
    particle: neutron (ID=2112); momentum: 2 GeV/c; trajectory with 9 points at ( 4 ; -3 ; -3 ) cm toward ( 0 ; 1 ; 0 ) with momentum 2 GeV/c
    ends at ( 4 ; 5 ; -3 ) cm toward ( 0 ; 1 ; 0 ) with momentum 2 GeV/c
    
One remarkable feature of FHiCL configuration is employed in the module
`ParticleMaker`, which allows reading a sequence of structures from the
configuration file (`test_totallycheattracker.fcl` shows that).


## Questions?  #################################################################

If you have any question about the example, please contact its author.
This section will be populated with questions and their answers.


## Change log  #################################################################

Version 1.0: December 26, 2017 (petrillo@fnal.gov)
  original version



[art wiki]:
  <https://cdcvs.fnal.gov/redmine/projects/art/wiki/Data_Product_Design_Guide>
[LArSoft wiki]:
  <https://cdcvs.fnal.gov/redmine/projects/larsoft/wiki/The_rules_and_guidelines#Coding-Conventions>
