#  Tutorial: tracking the throw of an exception {#CatchException_README}

| Example name:   | CatchException                        |
| --------------- | ------------------------------------- |
| Type:           | Tutorial (with module)                |
| Author:         | Gianluca Petrillo (petrillo@fnal.gov) |
| Created on:     | March 21, 2018                        |
| Version:        | 1.0                                   |


This is a tutorial on how to track down where an exception is thrown in the
code.


### How to use this document                                                 ###

After you have compiled the code in the usual way, read the introduction first
and then just follow one of the tutorials. If you have a real use case, you may
start with the first tutorial, and if it does not work go to the second, then
the third, then the last.
Or go directly to the last, which is more annoying but the safest.

Available tutorials:

* track the only exception in the job
* track one of the few exceptions in the job
* track one exception after many exceptions of a different type in the job
* track one exception of many exceptions of the same type in the job

For every questions, answered or not here, you are strongly encouraged to
contact the example's author (contact information is at the top of this file).

And, if you want to have a bit more printer friendly format, know this text file
is written in markdown format and you can convert it to something else with:
    
    pandoc -s -S --toc -o README.html README.md
    pandoc -s -S --toc -o README.pdf README.md
    
et cetera.

## Introduction to the tutorials  ##############################################

In these tutorials we try to track down the location in the code where a fatal
exception is thrown.

We'll use a debugger. The use of a `debug` version of `larexamples` will
improve the debugging experience, but it's not required (in fact, for this
tutorial I am using `prof` myself).
We are going to run a job which eventually executes a module that is in fact
designed to throw an exception for our own entertainment.

I'll use GDB, because it's almost everywhere. In one of the places where that is
not, LLDB is available instead, and there are maps of GDB to LLDB commands all
over the Internet (and LLDB kindly implements GDB-like aliases for many of the
common commands). If you are working on a system with UPS, set up the most
recent version you have available (check with `ups list -aK+ gdb | sort`); for
example, at the time of writing:
    
    setup gdb v8_0_1
    


## Track the only exception in the job  ########################################

In this tutorial we track the fatal exception out of `exploder_badalloc.fcl`.
Let's start with running the job:
    
    lar -c exploder_badalloc.fcl
    
which in my session ends like:
    
    %MSG-s ArtException:  PostEndJob 21-Mar-2018 20:42:58 CDT ModuleEndJob
    cet::exception caught in art
    ---- OtherArt BEGIN
      ---- EventProcessorFailure BEGIN
        EventProcessor: an exception occurred during current event processing
        ---- EventProcessorFailure BEGIN
          EndPathExecutor: an exception occurred during current event processing
          ---- ScheduleExecutionFailure BEGIN
            Path: ProcessingStopped.
            ---- BadAlloc BEGIN
              A std::bad_alloc exception occurred during a call to the module larexamples/DebuggingExamples/CatchException/Exploder/exploder run: 1 subRun: 0 event: 1
              The job has probably exhausted the virtual memory available to the process.
            ---- BadAlloc END
            Exception going through path end_path
          ---- ScheduleExecutionFailure END
        ---- EventProcessorFailure END
      ---- EventProcessorFailure END
    ---- OtherArt END
    %MSG
    Art has completed and will exit with status 1.
    
So, _art_ has completed with status `1`, which means error (everything that is
not `0` means error, after all). And it seems there is a memory allocation that
was beyond the availability of the system.
Let's fire up a debugger: let's use the same command line as before, but with GDB:
    
    gdb --args lar -c exploder_badalloc.fcl
    
If we run (which I recommend), by issuing the `run` command, we'll get the same
result as before. Not very helpful so far...

We can tell GDB to stop every time an exception is thrown. After all, exceptions
are used only in case of... exceptional conditions, right?
    
    catch throw
    run
    
Note that after the first command, GDB tells us something like
`Catchpoint 1 (throw)`. The `1` here is important: GDB identifies all the points
we ask it to interrupt the program (_breakpoints_) with a unique number.
We are dropped at a point like:
    
    %MSG-i MF_INIT_OK:  Early 21-Mar-2018 20:46:42 CDT JobSetup
    Messagelogger initialization complete.
    %MSG
    Begin processing the 1st record. run: 1 subRun: 0 event: 1 at 21-Mar-2018 20:46:44 CDT
    Now allocating: 17592186044415 x 1048576 bytes
    
    Catchpoint 1 (exception thrown), __cxxabiv1::__cxa_throw (obj=0x7fffdc000940, tinfo=0x7ffff0aba770 <typeinfo for std::bad_alloc>, dest=0x7ffff07d5a60 <std::bad_alloc::~bad_alloc()>)
        at ../../.././libstdc++-v3/libsupc++/eh_throw.cc:63
    63      ../../.././libstdc++-v3/libsupc++/eh_throw.cc: No such file or directory.
    
GDB is telling us that something has thrown an exception from line `63` of
source file `../../.././libstdc++-v3/libsupc++/eh_throw.cc`. Huh?! Well, let's
see where that comes from. Note that it's promising, since it also shows that we
are in the destructor of `std::bad_alloc`, which sounds related to our error
message.

If you are following this tutorial for a real debugging case and you find
instead that this exception is not what you are looking for, check out the next
tutorial.

Provided that this is indeed the exception we want, we ask an extract of the
stack backtrace, that is the list of calls that led to the function throwing the
exception. Since this list can be long, we start with limiting it to the 10
lowest entries:
    
    backtrace 10
    
(short: `bt 10`)
    
    #0  __cxxabiv1::__cxa_throw (obj=0x7fffdc000940, tinfo=0x7ffff0aba770 <typeinfo for std::bad_alloc>, dest=0x7ffff07d5a60 <std::bad_alloc::~bad_alloc()>)
        at ../../.././libstdc++-v3/libsupc++/eh_throw.cc:63
    #1  0x00007ffff07d7d5c in operator new (sz=sz@entry=18446744073708503040) at ../../.././libstdc++-v3/libsupc++/new_op.cc:54
    #2  0x00007fffe12630b1 in __gnu_cxx::new_allocator<std::array<unsigned char, 1048576ul> >::allocate (this=<synthetic pointer>, __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/ext/new_allocator.h:104
    #3  std::allocator_traits<std::allocator<std::array<unsigned char, 1048576ul> > >::allocate (__a=<synthetic pointer>..., __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/alloc_traits.h:436
    #4  std::_Vector_base<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::_M_allocate (this=<synthetic pointer>, __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:170
    #5  std::vector<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::_M_default_append (__n=17592186044415, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/vector.tcc:557
    #6  std::vector<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::resize (__new_size=17592186044415, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:677
    #7  lar::example::Exploder::throwBadAlloc () at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:181
    #8  0x00007fffe1263165 in lar::example::Exploder::analyze (this=0x4f8a080)
        at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:120
    #9  0x00007ffff6a2ce7d in art::EDAnalyzer::doEvent (this=0x4f8a080, ep=..., cpc=..., counts=...)
        at /scratch/workspace/art-release-build/SLF7/prof/build/art/v2_10_03/src/art/Framework/Core/EDAnalyzer.cc:29
    
Reading the lines one by one, something starts to make sense at the level #6,
where `std::vector::resize` is mentioned: it's a tyep of vector we use in our
code, and we wrote a `resize()` call after all. Even better, at level #7 we are
informed that the function `lar::example::Exploder::throwBadAlloc` calls it at
line 181.
We can ask GDB to give us a snapshot of where that is:
    
    frame 7
    list
    
The first command already shows the incriminated line, and the second gives it
10 lines of context:
    
    176       std::vector<OneMebibyte> manyMebibytes;
    177
    178       // this is allowed, but we don't have enough memory
    179       mf::LogVerbatim("Exploder") << "Now allocating: " << manyMebibytes.max_size()
    180         << " x " << sizeof(OneMebibyte) << " bytes";
    181       manyMebibytes.resize(manyMebibytes.max_size());
    182
    183     } // lar::example::Exploder::throwBadAlloc()
    184
    185
    
What did we do?! If we go down the stack one level to frame #6 (`down`), we see
    
    #6  std::vector<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::resize (__new_size=17592186044415, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:677
    677               _M_default_append(__new_size - size());
    
that shows the first argument of the vector to be `__new_size=17592186044415`,
which is... a lot. That should not come to a surprise, since the debugger had
shown this information already in the backtrace listing above, and even our own
output was declaring the attempt to allocate `17592186044415` 1-MiB objects.
Oh well. Problem tracked, anyway (exit the debugger with `quit`).

We can verify that the value of `i` is `2` (`print i`), but other operations
might be less successful: for example, if I ask to `print intData` I get:
    
    $2 = <optimized out>
    
Using the `debug` version (again, this is `prof`), this might be more
successful, or not, depending on the choices the compiler did.
Anyway, the purpose of this tutorial is to find where the exception was thrown,
and we did: it's the third iteration (`i = 2`) of the loop in
`lar::example::Exploder::analyze`.

### TL;DR: summary

The easiest way to find where an exception is thrown is to ask GDB
    
    catch throw
    run
    
It will stop at the first exception thrown.


## Track one of the few exceptions in the job  #################################

Let's now debug `exploder_outofrange.fcl`:
    
    lar -c exploder_outofrange.fcl
    
This throws like:
    
    %MSG-s ArtException:  PostEndJob 21-Mar-2018 20:57:36 CDT ModuleEndJob
    cet::exception caught in art
    ---- OtherArt BEGIN
      ---- EventProcessorFailure BEGIN
        EventProcessor: an exception occurred during current event processing
        ---- EventProcessorFailure BEGIN
          EndPathExecutor: an exception occurred during current event processing
          ---- ScheduleExecutionFailure BEGIN
            Path: ProcessingStopped.
            ---- StdException BEGIN
              A std::exception occurred during a call to the module larexamples/DebuggingExamples/CatchException/Exploder/exploder run: 1 subRun: 0 event: 1
              and cannot be repropagated.
              Previous information:
              vector::_M_range_check: __n (which is 5) >= this->size() (which is 5)
            ---- StdException END
            Exception going through path end_path
          ---- ScheduleExecutionFailure END
        ---- EventProcessorFailure END
      ---- EventProcessorFailure END
    ---- OtherArt END
    %MSG
    Art has completed and will exit with status 1.
    
The output tells us we are looking for a `std::exception`, and there is some
blattering about range checks. Let's go to the debugger:
    
    gdb --args lar -c exploder_outofrange.fcl
    
We `run` to confirm we can reproduce the error (we can), and then we catch the
throws and rerun:
    
    catch throw
    run
    
It turns out, this time we are looking for range checks, but we are dropped at:
    
    Begin processing the 1st record. run: 1 subRun: 0 event: 1 at 21-Mar-2018 20:59:57 CDT
    Now allocating: 17592186044415 x 1048576 bytes

    Catchpoint 1 (exception thrown), __cxxabiv1::__cxa_throw (obj=0x7fffdc000940, tinfo=0x7ffff0aba770 <typeinfo for std::bad_alloc>, dest=0x7ffff07d5a60 <std::bad_alloc::~bad_alloc()>)
        at ../../.././libstdc++-v3/libsupc++/eh_throw.cc:63
    63      ../../.././libstdc++-v3/libsupc++/eh_throw.cc: No such file or directory.
    
It's about allocations... is it *it*? In fact, it is not: a backtrace will show
we are in a place similar to the previous tutorial:
    
    #0  __cxxabiv1::__cxa_throw (obj=0x7fffdc000940, tinfo=0x7ffff0aba770 <typeinfo for std::bad_alloc>, dest=0x7ffff07d5a60 <std::bad_alloc::~bad_alloc()>)
        at ../../.././libstdc++-v3/libsupc++/eh_throw.cc:63
    #1  0x00007ffff07d7d5c in operator new (sz=sz@entry=18446744073708503040) at ../../.././libstdc++-v3/libsupc++/new_op.cc:54
    #2  0x00007fffe12630b1 in __gnu_cxx::new_allocator<std::array<unsigned char, 1048576ul> >::allocate (this=<synthetic pointer>, __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/ext/new_allocator.h:104
    #3  std::allocator_traits<std::allocator<std::array<unsigned char, 1048576ul> > >::allocate (__a=<synthetic pointer>..., __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/alloc_traits.h:436
    #4  std::_Vector_base<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::_M_allocate (this=<synthetic pointer>, __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:170
    #5  std::vector<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::_M_default_append (__n=17592186044415, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/vector.tcc:557
    #6  std::vector<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::resize (__new_size=17592186044415, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:677
    #7  lar::example::Exploder::throwBadAlloc () at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:181
    #8  0x00007fffe126313a in lar::example::Exploder::analyze (this=0x4f89890)
        at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:115
    #9  0x00007ffff6a2ce7d in art::EDAnalyzer::doEvent (this=0x4f89890, ep=..., cpc=..., counts=...)
        at /scratch/workspace/art-release-build/SLF7/prof/build/art/v2_10_03/src/art/Framework/Core/EDAnalyzer.cc:29
    
but the error we get is different from the one in that tutorial. What is
happening?
This time, the program can actually manage the exception, but GDB does not know
anything of that. A look at the level #8 will show that we are at:
    
    110       //
    111       // std::length_error
    112       //
    113       if (fManageBadAlloc) {
    114         try {
    115           throwBadAlloc();
    116         }
    117         catch (std::bad_alloc const&) {}
    118       }
    119       else {
    
which is in fact a different line of code than the one in the previous tutorial;
we see that we are at line 115 (from the backtrace information), not at line 120
as before, and we also see that the next thing the program was going to do was
to catch that exception and "manage" it.
Ok... red herring number one, and also lesson number one: whichever exception is
thrown, GDB will kindly stop for us to take action.
We can try our luck and tell GDB to `continue` and see what happens:
    
    Continuing.
    Starting TOOR iteration #0
    Starting TOOR iteration #1
    Starting TOOR iteration #2
    Starting TOOR iteration #3
    Starting TOOR iteration #4
    Starting TOOR iteration #5

    Catchpoint 1 (exception thrown), __cxxabiv1::__cxa_throw (obj=obj@entry=0x7fffdc0010a0, tinfo=0x7ffff0abba98 <typeinfo for std::out_of_range>,
        dest=0x7ffff07ecdb0 <std::out_of_range::~out_of_range()>) at ../../.././libstdc++-v3/libsupc++/eh_throw.cc:63
    63      ../../.././libstdc++-v3/libsupc++/eh_throw.cc: No such file or directory.
    
This is output we had also in our run without the debugger, and then we are left
at:
    
    #0  __cxxabiv1::__cxa_throw (obj=obj@entry=0x7fffdc0010a0, tinfo=0x7ffff0abba98 <typeinfo for std::out_of_range>, dest=0x7ffff07ecdb0 <std::out_of_range::~out_of_range()>)
        at ../../.././libstdc++-v3/libsupc++/eh_throw.cc:63
    #1  0x00007ffff0800135 in std::__throw_out_of_range_fmt (__fmt=__fmt@entry=0x7fffe1271460 "vector::_M_range_check: __n (which is %zu) >= this->size() (which is %zu)")
        at ../../../.././libstdc++-v3/src/c++11/functexcept.cc:104
    #2  0x00007fffe1262e94 in std::vector<int, std::allocator<int> >::_M_range_check (__n=5, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:804
    #3  std::vector<int, std::allocator<int> >::at (__n=5, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:825
    #4  lar::example::Exploder::throwOutOfRange () at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:162
    #5  0x00007fffe1263173 in lar::example::Exploder::analyze (this=0x4f89890)
        at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:133
    #6  0x00007ffff6a2ce7d in art::EDAnalyzer::doEvent (this=0x4f89890, ep=..., cpc=..., counts=...)
        at /scratch/workspace/art-release-build/SLF7/prof/build/art/v2_10_03/src/art/Framework/Core/EDAnalyzer.cc:29
    [...]
    
Again reading the backtrace, we see our code at #3 and it's executing the
`std::vector::at` method, which is in fact known to perform range check and
throw a `std::out_of_range` exception if that check fails. Focusing on the stack
frame where `at()` is called (`frame 4`), we can `list` the code:
    
    157       int intTotal = 0;
    158       for (unsigned int i = 0; i < 10; ++i) {
    159         mf::LogVerbatim("Exploder") << "Starting TOOR iteration #" << i;
    160
    161         // possible std::out_of_range throw
    162         intTotal += intData.at(i);
    163
    164       } // for
    165       mf::LogVerbatim("Exploder") << "TOOR iterations completed.";
    166
    
What are we doing? we can ask the debugger to `print i`, which we are told has
value `5`. We can also ask to print the vector itself (`print intData`) or its
size (`print intData.size()`), but the level of success may vary a lot. For
example, in my optimised code (`prof` qualifier), the former gives me an
unfriendly view of the vector (showing its three internal pointers) while the
latter fails miserably with:
    
    Can't take address of "intData" which isn't an lvalue.
    
If you know about `std::vector`, you can get the size with the expression:
`print intData._M_impl._M_finish - intData._M_impl._M_start` (that is, number of
elements between the pointers of the end and the begin of the data); in my case
that also fails because `intData._M_impl._M_finish` is also "optimized out"
(`intData._M_impl._M_start` is not... bah).
Anyway, this is where we wanted to be, and where our bug is.

### TL;DR: summary

The easiest way to find where an exception is thrown is to ask GDB
    
    catch throw
    run
    
It will stop at the first exception thrown. If that's not the one that hit you,
then you can ask GDB to
    
    continue
    
until you are dropped at the right one.


## Track one exception after many exceptions of a different type in the job  ###

Let's now debug `exploder_disturbed_outofrange.fcl`:
    
    lar -c exploder_disturbed_outofrange.fcl
    
This throws like:
    
    %MSG-s ArtException:  PostEndJob 22-Mar-2018 10:07:02 CDT ModuleEndJob
    cet::exception caught in art
    ---- OtherArt BEGIN
      ---- EventProcessorFailure BEGIN
        EventProcessor: an exception occurred during current event processing
        ---- EventProcessorFailure BEGIN
          EndPathExecutor: an exception occurred during current event processing
          ---- ScheduleExecutionFailure BEGIN
            Path: ProcessingStopped.
            ---- StdException BEGIN
              A std::exception occurred during a call to the module larexamples/DebuggingExamples/CatchException/Exploder/exploder run: 1 subRun: 0 event: 1
              and cannot be repropagated.
              Previous information:
              vector::_M_range_check: __n (which is 5) >= this->size() (which is 5)
            ---- StdException END
            Exception going through path end_path
          ---- ScheduleExecutionFailure END
        ---- EventProcessorFailure END
      ---- EventProcessorFailure END
    ---- OtherArt END
    %MSG
    Art has completed and will exit with status 1.
    
We have seen this exception in a previous tutorial. If we follow that one:
    
    catch throw
    run
    
we are dropped at:
    
    89
    90      //------------------------------------------------------------------------------
    91      void lar::example::Disturbance::throwArtException() {
    92
    93        throw art::Exception(art::errors::LogicError)
    94          << "I want to annoy you.\n";
    95
    96      } // lar::example::Disturbance::throwArtException()
    97
    
(that is `frame 2`). That is not the exception we are looking for. If we hit
`continue`, we are driven to... the same place, again and again.
Then we get out of patience very quickly, and the next approach is due.
This is in fact the realistic situation we encounter with some XML parsing
library that LArSoft indirectly depends on.
That next approach is to use a breakpoint on the call to all the constructors of
the exception we are hunting: before being thrown, the exception object must
be constructed! Here we fetch some additional knowledge: that exception is a C++
STL exception known as `std::out_of_range`. It can be recognised by the error
message. 
So we set a new one on the constructors of `std::out_of_range`, and we also
remove the first breakpoint that we set with `catch throw` earlier, to go beyond
the stalling point:
    
    break std::out_of_range::out_of_range
    delete 1
    
(`1` is what GDB told us the first breakpoint number was, and now it tells us
also that the new one is `2`). In this case, since we have already loaded the
program, GDB already knows of the class `std::out_of_range` and it tells us:
    
    Breakpoint 2 at 0x7fffee44bc30 (11 locations)
    
meaning that there are many constructors for that class (you can see that with
`info breakpoints`). If we hadn't loaded and run the program, it might happen
that the debugger tells us it does not know about that symbol
`std::out_of_range::out_of_range`, and asks us if we want it to keep an eye
open and set the breakpoint as soon as that symbol is loaded. In general, that's
what we want, but be warned that if we misspelled the function name (not at all
unlikely), no break will happen and we will not be warned about that.

Since we are already in the middle of the debugging, we can `continue` after
having rearranged our breakpoints as described above. Or we can `run` again from
the beginning. We here choose:
    
    Continuing.
    Now allocating: 17592186044415 x 1048576 bytes
    Starting TOOR iteration #0
    Starting TOOR iteration #1
    Starting TOOR iteration #2
    Starting TOOR iteration #3
    Starting TOOR iteration #4
    Starting TOOR iteration #5

    Breakpoint 2, 0x00007ffff07d3b80 in std::out_of_range::out_of_range(char const*)@plt ()
      from /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/lib64/libstdc++.so.6    
    
and we are left at the constructor as requested. The backtrace again shows a
call to `std::vector::at` at frame #3:
    
    #0  0x00007ffff07d3b80 in std::out_of_range::out_of_range(char const*)@plt () from /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/lib64/libstdc++.so.6
    #1  0x00007ffff080011f in std::__throw_out_of_range_fmt (__fmt=__fmt@entry=0x7fffe1271460 "vector::_M_range_check: __n (which is %zu) >= this->size() (which is %zu)")
        at ../../../.././libstdc++-v3/src/c++11/functexcept.cc:104
    #2  0x00007fffe1262e94 in std::vector<int, std::allocator<int> >::_M_range_check (__n=5, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:804
    #3  std::vector<int, std::allocator<int> >::at (__n=5, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:825
    #4  lar::example::Exploder::throwOutOfRange ()
        at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:162
    
ant it shows that it comes from a method in `frame 4`:
    
    157       int intTotal = 0;
    158       for (unsigned int i = 0; i < 10; ++i) {
    159         mf::LogVerbatim("Exploder") << "Starting TOOR iteration #" << i;
    160
    161         // possible std::out_of_range throw
    162         intTotal += intData.at(i);
    163
    164       } // for
    165       mf::LogVerbatim("Exploder") << "TOOR iterations completed.";
    166
    
Same point as in a previous tutorial, but this time it was harder to get there.
And yet we did!

### TL;DR: summary

If the easiest way fails because a lot of exceptions are thrown before the one
we seek, we can set a breakpoint on the constructor of the exception class,
which we need to know or guess. For example, if the exception is
`std::out_of_range`:
    
    break std::out_of_range::out_of_range
    
The old throw catchpoint can be deleted as any other breakpoint, by knowing its
index (e.g. `delete 1`).


## Track one exception of many exceptions of the same type in the job  #########

Let's now debug `exploder_disturbed_artexception.fcl`:
    
    lar -c exploder_disturbed_artexception.fcl
    
This throws like:
    
    %MSG-s ArtException:  PostEndJob 22-Mar-2018 10:29:38 CDT ModuleEndJob
    cet::exception caught in art
    ---- OtherArt BEGIN
      ---- EventProcessorFailure BEGIN
        EventProcessor: an exception occurred during current event processing
        ---- EventProcessorFailure BEGIN
          EndPathExecutor: an exception occurred during current event processing
          ---- ScheduleExecutionFailure BEGIN
            Path: ProcessingStopped.
            ---- LogicError BEGIN
              I hate the world and I am vengeful.
              cet::exception going through module larexamples/DebuggingExamples/CatchException/Exploder/exploder run: 1 subRun: 0 event: 1
            ---- LogicError END
            Exception going through path end_path
          ---- ScheduleExecutionFailure END
        ---- EventProcessorFailure END
      ---- EventProcessorFailure END
    ---- OtherArt END
    %MSG
    Art has completed and will exit with status 1.
    
This exception is a `cet::exception`. Following the previous tutorials, we find
ourselves in the situation where we keep being dropped at the wrong exception:
    
    89
    90      //------------------------------------------------------------------------------
    91      void lar::example::Disturbance::throwArtException() {
    92
    93        throw art::Exception(art::errors::LogicError)
    94          << "I want to annoy you.\n";
    95
    96      } // lar::example::Disturbance::throwArtException()
    97
    98
    
We know it's the wrong exception because the error message the code prints does
not match the one we get on the screen when running. _art_ tells us the
exception we seek is of type `cet::exception`. It turns out it's actually a
descendent of that, `art::Exception`, but we stick to what we know:
    
    break cet::exception::exception
    delete 1
    
sets a whole load of breakpoints, and when we hit `continue`...
    
    88
    89
    90      //------------------------------------------------------------------------------
    91      void lar::example::Disturbance::throwArtException() {
    92
    93        throw art::Exception(art::errors::LogicError)
    94          << "I want to annoy you.\n";
    95
    96      } // lar::example::Disturbance::throwArtException()
    97
    
we are left here again. That ain't good. At this point we need to get more
creative. The idea is to let the program run undisturbed until we enter the part
of the program we believe the exception is being thrown in. The error message
mentions `larexamples/DebuggingExamples/CatchException/Exploder/exploder`.
This is in fact the module throwing the exception (usually, no path is
specified and it would be just `Exploder/exploder`).
The command `lar --print-description Exploder` will tell us the path to the
source code and that the module is an analyzer. Then we can delete all the
breakpoints (`delete`, then confirm) and set a single breakpoint where the
action of the module starts: `break lar::example::Exploder::analyze` (we learn
the namespace from the source code). Incidentally, GDB usually supports
automatic completion by hitting the TAB key, but that may take *very* long
time. If the library where `Exploder` module is hasn't been loaded yet, we'll be
asked whether we want to make the breakpoint "pending", which we do want.
We then `run` or `continue`, and we'll hit that breakpoint soon:
    
    Breakpoint 5, lar::example::Exploder::analyze (this=0x4f922d0)
        at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:108
    108     void lar::example::Exploder::analyze(art::Event const&) {
    
Att his point we can start again with the old tricks: `catch throw`, or
`break cet::exception::exception`, and then `continue`.
For this tutorial, I did the former, and I landed to:
    
    #0  __cxxabiv1::__cxa_throw (obj=0x7fffdc000940, tinfo=0x7ffff0aba770 <typeinfo for std::bad_alloc>, dest=0x7ffff07d5a60 <std::bad_alloc::~bad_alloc()>)
        at ../../.././libstdc++-v3/libsupc++/eh_throw.cc:63
    #1  0x00007ffff07d7d5c in operator new (sz=sz@entry=18446744073708503040) at ../../.././libstdc++-v3/libsupc++/new_op.cc:54
    #2  0x00007fffe12630b1 in __gnu_cxx::new_allocator<std::array<unsigned char, 1048576ul> >::allocate (this=<synthetic pointer>, __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/ext/new_allocator.h:104
    #3  std::allocator_traits<std::allocator<std::array<unsigned char, 1048576ul> > >::allocate (__a=<synthetic pointer>..., __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/alloc_traits.h:436
    #4  std::_Vector_base<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::_M_allocate (this=<synthetic pointer>, __n=17592186044415)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:170
    #5  std::vector<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::_M_default_append (__n=17592186044415, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/vector.tcc:557
    #6  std::vector<std::array<unsigned char, 1048576ul>, std::allocator<std::array<unsigned char, 1048576ul> > >::resize (__new_size=17592186044415, this=<synthetic pointer>)
        at /cvmfs/larsoft.opensciencegrid.org/products/gcc/v6_4_0/Linux64bit+3.10-2.17/include/c++/6.4.0/bits/stl_vector.h:677
    #7  lar::example::Exploder::throwBadAlloc () at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:181
    #8  0x00007fffe126313a in lar::example::Exploder::analyze (this=0x4f922d0)
        at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:115
    #9  0x00007ffff6a2ce7d in art::EDAnalyzer::doEvent (this=0x4f922d0, ep=..., cpc=..., counts=...)
        at /scratch/workspace/art-release-build/SLF7/prof/build/art/v2_10_03/src/art/Framework/Core/EDAnalyzer.cc:29
    
that at `frame 7` shows our code throwing the exception:
    
    176       std::vector<OneMebibyte> manyMebibytes;
    177
    178       // this is allowed, but we don't have enough memory
    179       mf::LogVerbatim("Exploder") << "Now allocating: " << manyMebibytes.max_size()
    180         << " x " << sizeof(OneMebibyte) << " bytes";
    181       manyMebibytes.resize(manyMebibytes.max_size());
    182
    183     } // lar::example::Exploder::throwBadAlloc()
    184
    
This is a place we encountered already in a previous tutorial, and it is in fact
_not_ our target. It takes a couple more of `continue` to get to:
    
    #0  __cxxabiv1::__cxa_throw (obj=obj@entry=0x7fffdc000960,
        tinfo=0x7ffff7dd6768 <typeinfo for cet::coded_exception<art::errors::ErrorCodes, &art::ExceptionDetail::translate[abi:cxx11]>>,
        tinfo@entry=0x7fffe1476e78 <typeinfo for cet::coded_exception<art::errors::ErrorCodes, &art::ExceptionDetail::translate[abi:cxx11]>>,
        dest=0x7ffff7b426b0 <cet::coded_exception<art::errors::ErrorCodes, &art::ExceptionDetail::translate[abi:cxx11]>::~coded_exception()>,
        dest@entry=0x7fffe1264d60 <cet::coded_exception<art::errors::ErrorCodes, &art::ExceptionDetail::translate[abi:cxx11]>::~coded_exception()>)
        at ../../.././libstdc++-v3/libsupc++/eh_throw.cc:63
    #1  0x00007fffe1262435 in lar::example::Exploder::throwArtException ()
        at /scratch/petrillo/LArSoft/software/build/develop/prof/srcs/larexamples/larexamples/DebuggingExamples/CatchException/Exploder_module.cc:190
    #2  0x00007ffff6a2ce7d in art::EDAnalyzer::doEvent (this=0x4f922d0, ep=..., cpc=..., counts=...)
        at /scratch/workspace/art-release-build/SLF7/prof/build/art/v2_10_03/src/art/Framework/Core/EDAnalyzer.cc:29    
    
which at `frame 1` shows:
    
    185
    186     //------------------------------------------------------------------------------
    187     void lar::example::Exploder::throwArtException() {
    188
    189       throw art::Exception(art::errors::LogicError)
    190         << "I hate the world and I am vengeful.\n";
    191
    192     } // lar::example::Exploder::throwArtException()
    193
    
This finally matches the error message. To have a confirmation: if executing
`continue` again, we'll immediately get the failure that _art_ shows.

Note: instead of deleting the breakpoint to `cet::exception::exception`, it can
be disabled (`disable 2`) and then re-enabled when getting to the `analyze`
method (`enable 2`).

### TL;DR: summary

If there are many exceptions of the same type being thrown and caught, and we
are interested in the last one, which is of course not caught, we can run the
program without breakpoints until it reaches the code we suspect, which may be
the code of the module we are told in the error message. Information about the
module can be obtained by `lar --print-description ModuleClass`, and the
breakpoint can be set on its `produce`, `filter` or `analyze` (or `beginJob`,
etc.) method with `breakpoint ns::ModuleClass::produce` etc.



## Questions?  #################################################################

If you have any question about the example, please contact its author.
This section will be populated with questions and their answers.


## Change log  #################################################################

Version 1.0: March 21, 2018 (petrillo@fnal.gov)
  original version

