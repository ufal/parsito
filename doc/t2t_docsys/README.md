# T2T DocSys
[![Build Status](https://travis-ci.org/ufal/t2t_docsys.svg?branch=master)](https://travis-ci.org/ufal/t2t_docsys)

T2T DocSys is a small documentation system capable of producing plain text,
Markdown, HTML and PDF, in several versions (e.g. monolithic / page per section).
It is a slight modification of txt2tags document generator.

Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty of
Mathematics and Physics, Charles University in Prague, Czech Republic.

T2T DocSys repository http://github.com/ufal/t2t_docsys is hosted on GitHub.

## Usage

- Include Makefile.include.

- Following functions are defined:
  - `$(call t2t_docsys_txt,output,input,configs)`
  - `$(call t2t_docsys_md,output,input,configs)`
  - `$(call t2t_docsys_html,output,input,configs)`
  - `$(call t2t_docsys_pdf,output,input,configs)`  
  Create document with specified output name from given input t2t file.
  Uses (possible multiple) given configurations. Currently there are:
    - `manual`: base configuration which also includes "^%manual% " lines
    - `web`: base configuration which also includes "^%web% " lines
    - `striplevel`: addon configuration which only strip one level of headings
    - `html_addlevel`: addon configuration which only add level to HTML headings

- Following variables are defined:
  - `T2T_DOCSYS_OUTPUT_WILDCARDS`  
    Wildcards of files produced by t2t_docsys. There are quite a few because
    of calling pdflatex.

  - `T2T_DOCSYS_VERSION`  
    Current T2T DocSys version.

## Markup

Standard txt2tags http://txt2tags.org/docs.html markup is supported, with
several additions:
- UTF-8 encoding is default.
- Paragraphs are reflowed in plain text output.
- There can be local links in `[ link #anchor ]` format in verbatim code, but
  they are retained only in HTML and PDF output.
- An empty/nonempty line between para/list and para/verb is taken into account.
- Two or more spaces at end of line denote a line break.
