# Analysis example  {#AnalysisExample_README}

| Example name:   | AnalysisExample                             |
| --------------- | ------------------------------------------- |
| Type:           | LArSoft module                              |
| Author:         | Bill Seligman (seligman@nevis.columbia.edu) |
| Created on:     | August 7, 2017                              |


This directory is intended to be part of a soup-to-nuts example of how
to run a LArSoft program to perform an analysis task.

For directions on how to set up and compile this example, see the LArSoft wiki:
<https://cdcvs.fnal.gov/redmine/projects/larsoft/wiki/_AnalysisExample_>

The example program in package `AnalysisExample` does the following:

- reads a file of simulated and potentially partially reconstructed
  events;

- creates a few example histograms;

- creates two example n-tuples;

- has a couple of brief examples of how to use associations. 

There are comments (perhaps too many!) in every file to explain what
everything does.

If you want to understand what you're supposed to learn from all this,
read `ADDITIONAL_NOTES.md` in this directory.

The files with a `.md` extension (including this one) are written in
Markdown. If you'd like a pretty-printed version of these files, you
can use `pandoc`; e.g.:

```
pandoc -s -S -o README.html README.md
pandoc -s -S -o README.pdf  README.md
```
