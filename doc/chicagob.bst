%	Chicago Manual of Style, type B citations.
%  Adapted from the `apalike' style by KCB, 10-12-89.
%  Latest revision: 6-Apr-93
%
% Comment markers followed by >, i.e. %>, indicate comments from 
% `apalike.bst'.
%
%>Adapted from the `alpha' style, version 0.99a; for BibTeX version 0.99a.
%>Copyright (C) 1988, all rights reserved.
%>Copying of this file is allowed, provided that if you make any changes at all
%>you name it something other than `apalike.bst'.
%>This restriction helps ensure that all copies are identical.
%>Differences between this style and `alpha' are generally heralded by a `%'.
%>The file btxbst.doc has the documentation for alpha.bst.
%
% This style should be used with the `chicagob' LaTeX style (chicagob.sty).
% \cite's come out like "Border (1985)" or "Border (1985, Thm. 7.)", with 
% an optional [..] argument, in the text but there are no labels
% in the bibliography, and something like "1986." comes out immediately
% after the author.  Author (and editor) names appear as last name, comma,
% initials for the first author, but as initials, last name for subsequent
% authors.  A period separates authors from the year.
%>A `year' field is required for every entry, and so is either
%>an author (or in some cases, an editor) field or a key field.
%
% Other departures from `apalike'include:
% The address field is followed by a colon and then the publisher.
% The edition follows the title.
% Book titles are slanted, but capitalization follows the same style as
% articles. Journal titles are capitalized as well as slanted.
% There is no comma between the journal name and the volume number.
%
% Modifications to `apalike.bst' are annotated and marked with % KCB.
%
% Since style B is actually a family of styles, I don't care what you do
% to this file.
%
% History:
%>  15-sep-86	(SK,OP)	Original version, by Susan King and Oren Patashnik.
%>  10-nov-86	(OP)	Truncated the sort.key$ string to the correct length
%>			in bib.sort.order to eliminate error message.
%>  24-jan-88	(OP)	Updated for BibTeX version 0.99a, from alpha.bst 0.99a;
%>			apalike now sorts by author, then year, then title;
%>			THIS `apalike' VERSION DOES NOT WORK WITH BIBTEX 0.98i.
%   13-oct-89	(KCB)	Adapted to Chicago Manual of Style B family.
%   27-aug-92   (KCB)	Modified format.bvolume to use only `number'
%			field for series.
%    6-Apr-93	(KCB)	Added closing ) to labels
%    9-aug-94	(KCB)	Added macros for editions
%   11-aug-94	(KCB)	Changed to NFSS2 font selection commands
ENTRY
  { address
    author
    booktitle
    chapter
    edition
    editor
    howpublished
    institution
    journal
    key
%    month		not used in apalike
    note
    number
    organization
    pages
    publisher
    school
    series
    title
    type
    volume
    year
  }
  {}
  { label extra.label sort.label }

INTEGERS { output.state before.all mid.sentence after.sentence after.block }

FUNCTION {init.state.consts}
{ #0 'before.all :=
  #1 'mid.sentence :=
  #2 'after.sentence :=
  #3 'after.block :=
}

STRINGS { s t }

FUNCTION {output.nonnull}
{ 's :=
  output.state mid.sentence =
    { ", " * write$ }
    { output.state after.block =
	{ add.period$ write$
	  newline$
	  "\newblock " write$
	}
	{ output.state before.all =
	    'write$
	    { add.period$ " " * write$ }
	  if$
	}
      if$
      mid.sentence 'output.state :=
    }
  if$
  s
}

FUNCTION {output}
{ duplicate$ empty$
    'pop$
    'output.nonnull
  if$
}

FUNCTION {output.nonnull.space}		% KCB -- outputs space instead of comma
{ 's :=	
  output.state mid.sentence =
    { " " * write$ }
    { output.state after.block =
	{ add.period$ write$
	  newline$
	  "\newblock " write$
	}
	{ output.state before.all =
	    'write$
	    { add.period$ " " * write$ }
	  if$
	}
      if$
      mid.sentence 'output.state :=
    }
  if$
  s
}

FUNCTION {output.space}			% KCB -- outputs space instead of comma
{ duplicate$ empty$
    'pop$
    'output.nonnull.space
  if$
}

FUNCTION {output.nonnull.colon}			% This is probably
{ 's :=						% inefficient
  output.state mid.sentence =			% KCB
    { ": " * write$ }				% Change comma to colon
    { output.state after.block =		% See output.check.colon
	{ add.period$ write$
	  newline$
	  "\newblock " write$
	}
	{ output.state before.all =
	    'write$
	    { add.period$ " " * write$ }
	  if$
	}
      if$
      mid.sentence 'output.state :=
    }
  if$
  s
}

FUNCTION {output.check.colon}		% KCB -- outputs colon instead of comma
{ 't :=	
  duplicate$ empty$
    { pop$ "empty " t * " in " * cite$ * warning$ }
    'output.nonnull.colon
  if$
}

FUNCTION {output.check.space}	
{ 't :=	
  duplicate$ empty$
    { pop$ "empty " t * " in " * cite$ * warning$ }
    'output.nonnull.space		% KCB -- needed to eliminate comma
  if$					% after journal title
}


FUNCTION {output.check}
{ 't :=
  duplicate$ empty$
    { pop$ "empty " t * " in " * cite$ * warning$ }
    'output.nonnull
  if$
}

%>					apalike needs this function because
%>					the year has special punctuation;
%>					apalike ignores the month
FUNCTION {output.year.check}
{ add.period$				% KCB -- period after authors
    year empty$
    { "empty year in " cite$ * warning$ }
    { write$
      " " year * extra.label * "" *
      mid.sentence 'output.state :=
    }
  if$
}

FUNCTION {output.bibitem}
{ newline$
  "\bibitem[" write$
  label write$
  "]{" write$
  cite$ write$
  "}" write$
  newline$
  ""
  before.all 'output.state :=
}

FUNCTION {fin.entry}
{ add.period$
  write$
  newline$
}

FUNCTION {new.block}
{ output.state before.all =
    'skip$
    { after.block 'output.state := }
  if$
}

FUNCTION {new.sentence}
{ output.state after.block =
    'skip$
    { output.state before.all =
	'skip$
	{ after.sentence 'output.state := }
      if$
    }
  if$
}

FUNCTION {not}
{   { #0 }
    { #1 }
  if$
}

FUNCTION {and}
{   'skip$
    { pop$ #0 }
  if$
}

FUNCTION {or}
{   { pop$ #1 }
    'skip$
  if$
}

FUNCTION {new.block.checkb}
{ empty$
  swap$ empty$
  and
    'skip$
    'new.block
  if$
}

FUNCTION {field.or.null}
{ duplicate$ empty$
    { pop$ "" }
    'skip$
  if$
}

FUNCTION {emphasize}
{ duplicate$ empty$
    { pop$ "" }
    { "\textsl{" swap$ * "}" * }	% KCB -- Change \em to \textsl
  if$
}

INTEGERS { nameptr namesleft numnames }

FUNCTION {format.names}
{ 's :=
  #1 'nameptr :=
  s num.names$ 'numnames :=
  numnames 'namesleft :=
    { namesleft #0 > }
    { nameptr #1 >		% KCB last name first for first author only
		{ s nameptr "{f. }{vv~}{ll}{, jj}" format.name$ 't :=  }
		{ s nameptr "{vv~}{ll}{, jj}{, f.}" format.name$ 't :=   }
		if$
      nameptr #1 >
	{ namesleft #1 >
	    { ", " * t * }
	    { numnames #2 >
		{ "," * }
		'skip$
	      if$
	      t "others" =
		{ " et~al." * }
		{ " and " * t * }
	      if$
	    }
	  if$
	}
	't
      if$
      nameptr #1 + 'nameptr :=
      namesleft #1 - 'namesleft :=
    }
  while$
}

FUNCTION {format.authors}
{ author empty$
    { "" }
    { author format.names }
  if$
}

FUNCTION {format.key}			%> this function is just for apalike
{ empty$
    { key field.or.null }
    { "" }
  if$
}

FUNCTION {format.editors}
{ editor empty$
    { "" }
    { editor format.names
      editor num.names$ #1 >
	{ ", eds." * }			% KCB -- abbreviate eds.
	{ ", ed." * }			% KCB -- abbreviate ed.
      if$
    }
  if$
}

FUNCTION {format.title}
{ title empty$
    { "" }
    { title "t" change.case$ }
  if$
}

FUNCTION {n.dashify}
{ 't :=
  ""
    { t empty$ not }
    { t #1 #1 substring$ "-" =
	{ t #1 #2 substring$ "--" = not
	    { "--" *
	      t #2 global.max$ substring$ 't :=
	    }
	    {   { t #1 #1 substring$ "-" = }
		{ "-" *
		  t #2 global.max$ substring$ 't :=
		}
	      while$
	    }
	  if$
	}
	{ t #1 #1 substring$ *
	  t #2 global.max$ substring$ 't :=
	}
      if$
    }
  while$
}

FUNCTION {format.btitle}
{ title "t" change.case$			% KCB lower case titles
  emphasize
}

FUNCTION {tie.or.space.connect}
{ duplicate$ text.length$ #3 <
    { "~" }
    { " " }
  if$
  swap$ * *
}

FUNCTION {either.or.check}
{ empty$
    'pop$
    { "can't use both " swap$ * " fields in " * cite$ * warning$ }
  if$
}

FUNCTION {format.bvolume}
{ volume empty$
    { "" }
    { "volume" volume tie.or.space.connect
%     series empty$	%%%% USE NUMBER ONLY FOR SERIES!  KCB
%	'skip$
%	{ " of " * series emphasize * }
%      if$
%      "volume and number" number either.or.check
    }
  if$
}

FUNCTION {format.number.series}
{ volume empty$
    { number empty$
	{ series field.or.null }
	{ output.state mid.sentence =
	    { "number" }
	    { "Number" }
	  if$
	  number tie.or.space.connect
	  series empty$
	    { "there's a number but no series in " cite$ * warning$ }
	    { " in " * series * }
	  if$
	}
      if$
    }
    { "" }
  if$
}

FUNCTION {format.edition}
{ edition empty$
    { "" }
    { output.state mid.sentence =
	{ edition "l" change.case$ " ed." * }	% KCB abbreviate edition
	{ edition "t" change.case$ " ed." * }	% KCB abbreviate edition
      if$
    }
  if$
}

INTEGERS { multiresult }

FUNCTION {multi.page.check}
{ 't :=
  #0 'multiresult :=
    { multiresult not
      t empty$ not
      and
    }
    { t #1 #1 substring$
      duplicate$ "-" =
      swap$ duplicate$ "," =
      swap$ "+" =
      or or
	{ #1 'multiresult := }
	{ t #2 global.max$ substring$ 't := }
      if$
    }
  while$
  multiresult
}

FUNCTION {format.pages}
{ pages empty$
    { "" }
    { pages multi.page.check
	{ "pages" pages n.dashify tie.or.space.connect }
	{ "page" pages tie.or.space.connect }
      if$
    }
  if$
}

FUNCTION {format.vol.num.pages}
{ volume field.or.null
  number empty$
    'skip$
    { "(" number * ")" * *
      volume empty$
	{ "there's a number but no volume in " cite$ * warning$ }
	'skip$
      if$
    }
  if$
  pages empty$
    'skip$
    { duplicate$ empty$
	{ pop$ format.pages }
	{ ":" * pages n.dashify * }
      if$
    }
  if$
}

FUNCTION {format.chapter.pages}
{ chapter empty$
    'format.pages
    { type empty$
	{ "chapter" }
	{ type "l" change.case$ }
      if$
      chapter tie.or.space.connect
      pages empty$
	'skip$
	{ ", " * format.pages * }
      if$
    }
  if$
}

FUNCTION {format.in.ed.booktitle}
{ booktitle empty$
    { "" }
    { editor empty$
	{ "In " booktitle emphasize * }
	{ "In " format.editors * ", " * booktitle emphasize * }
      if$
    }
  if$
}

FUNCTION {format.thesis.type}
{ type empty$
    'skip$
    { pop$
      type "t" change.case$
    }
  if$
}

FUNCTION {format.tr.number}
{ type empty$
    { "Technical Report" }
    'type
  if$
  number empty$
    { "t" change.case$ }
    { number tie.or.space.connect }
  if$
}

FUNCTION {format.article.crossref}
{ "In"							%> this is for apalike
  " \cite{" * crossref * "}" *
}

FUNCTION {format.book.crossref}
{ volume empty$
    { "empty volume in " cite$ * "'s crossref of " * crossref * warning$
      "In "
    }
    { "Volume" volume tie.or.space.connect
      " of " *
    }
  if$
  "\cite{" * crossref * "}" *				%> this is for apalike
}

FUNCTION {format.incoll.inproc.crossref}
{ "In"							%> this is for apalike
  " \cite{" * crossref * "}" *
}

FUNCTION {article}
{ output.bibitem
  format.authors "author" output.check
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.title "title" output.check
  new.block
  crossref missing$
    { journal emphasize "journal" output.check
      format.vol.num.pages output.space
    }
    { format.article.crossref output.nonnull
      format.pages output
    }
  if$
  new.block
  note output
  fin.entry
}

FUNCTION {book}
{ output.bibitem
  author empty$
    { format.editors "author and editor" output.check
        editor format.key output
    }
    { format.authors output.nonnull
      crossref missing$
	{ "author and editor" editor either.or.check }
	'skip$
      if$
    }
  if$
  output.year.check				%> special for apalike
  new.block
  format.btitle "title" output.check
  format.edition output				% KCB -- edition comes after
  crossref missing$				% title
    { format.bvolume output
      new.block
      format.number.series output
      new.sentence
      address output				% KCB -- put city first
      publisher "publisher" output.check.colon	% KCB -- follow by colon
    }
    { new.block
      format.book.crossref output.nonnull
    }
  if$
  new.block
  note output
  fin.entry
}

FUNCTION {booklet}
{ output.bibitem
  format.authors output
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.title "title" output.check
  new.block
  howpublished output
  address output
  new.block
  note output
  fin.entry
}

FUNCTION {inbook}
{ output.bibitem
  author empty$
    { format.editors "author and editor" output.check
      editor format.key output
    }
    { format.authors output.nonnull
      crossref missing$
	{ "author and editor" editor either.or.check }
	'skip$
      if$
    }
  if$
  output.year.check				%> special for apalike
  new.block
  format.btitle "title" output.check
  crossref missing$
    { format.bvolume output
      format.chapter.pages "chapter and pages" output.check
      new.block
      format.number.series output
      new.sentence
      publisher "publisher" output.check
      address output
    }
    { format.chapter.pages "chapter and pages" output.check
      new.block
      format.book.crossref output.nonnull
    }
  if$
  format.edition output
  new.block
  note output
  fin.entry
}

FUNCTION {incollection}
{ output.bibitem
  format.authors "author" output.check
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.title "title" output.check
  new.block
  crossref missing$
    { format.in.ed.booktitle "booktitle" output.check
      format.bvolume output
      format.number.series output
      format.chapter.pages output
      new.sentence
      publisher "publisher" output.check
      address output
      format.edition output
    }
    { format.incoll.inproc.crossref output.nonnull
      format.chapter.pages output
    }
  if$
  new.block
  note output
  fin.entry
}

FUNCTION {inproceedings}
{ output.bibitem
  format.authors "author" output.check
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.title "title" output.check
  new.block
  crossref missing$
    { format.in.ed.booktitle "booktitle" output.check
      format.bvolume output
      format.number.series output
      format.pages output
      address output					%> for apalike
      new.sentence					%> there's no year
      organization output				%> here so things
      publisher output					%> are simpler
    }
    { format.incoll.inproc.crossref output.nonnull
      format.pages output
    }
  if$
  new.block
  note output
  fin.entry
}

FUNCTION {conference} { inproceedings }

FUNCTION {issue}		%%% KCB special issue of a journal
{ output.bibitem
  format.editors output
  editor format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.btitle "title" output.check
 new.block
  crossref missing$
    { journal emphasize "journal" output.check
      format.vol.num.pages output.space
    }
    { format.article.crossref output.nonnull
      format.pages output
    }
  if$
  new.block
  note output
  fin.entry
}

FUNCTION {manual}
{ output.bibitem
  format.authors output
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.btitle "title" output.check
  organization address new.block.checkb
  organization output
  address output
  format.edition output
  new.block
  note output
  fin.entry
}

FUNCTION {mastersthesis}
{ output.bibitem
  format.authors "author" output.check
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.title "title" output.check
  new.block
  "Master's thesis" format.thesis.type output.nonnull
  school "school" output.check
  address output
  new.block
  note output
  fin.entry
}

FUNCTION {misc}
{ output.bibitem
  format.authors output
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.title output
  new.block
  howpublished output
  new.block
  note output
  fin.entry
}

FUNCTION {phdthesis}
{ output.bibitem
  format.authors "author" output.check
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.btitle "title" output.check
  new.block
  "PhD thesis" format.thesis.type output.nonnull
  school "school" output.check
  address output
  new.block
  note output
  fin.entry
}

FUNCTION {proceedings}
{ output.bibitem
  format.editors output
  editor format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.btitle "title" output.check
  format.bvolume output
  format.number.series output
  address output				%> for apalike
  new.sentence					%> we always output
  organization output				%> a nonempty organization
  publisher output				%> here
  new.block
  note output
  fin.entry
}

FUNCTION {techreport}
{ output.bibitem
  format.authors "author" output.check
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.title "title" output.check
  new.block
  format.tr.number output.nonnull
  institution "institution" output.check
  address output
  new.block
  note output
  fin.entry
}

FUNCTION {unpublished}
{ output.bibitem
  format.authors "author" output.check
  author format.key output				%> special for
  output.year.check					%> apalike
  new.block
  format.title "title" output.check
  new.block
  note "note" output.check
  fin.entry
}

FUNCTION {default.type} { misc }

MACRO {jan} {"January"}

MACRO {feb} {"February"}

MACRO {mar} {"March"}

MACRO {apr} {"April"}

MACRO {may} {"May"}

MACRO {jun} {"June"}

MACRO {jul} {"July"}

MACRO {aug} {"August"}

MACRO {sep} {"September"}

MACRO {oct} {"October"}

MACRO {nov} {"November"}

MACRO {dec} {"December"}

% Macros for editions
MACRO {first} {"1st "}
MACRO {second} {"2d "}
MACRO {third} {"3d "}
MACRO {fourth} {"4th "}
MACRO {fifth} {"5th "}
MACRO {sixth} {"6th "}
MACRO {seventh} {"7th "}
MACRO {eighth} {"8th "}
MACRO {ninth} {"9th "}
MACRO {tenth} {"10th "}

READ

FUNCTION {sortify}
{ purify$
  "l" change.case$
}

INTEGERS { len }

FUNCTION {chop.word}
{ 's :=
  'len :=
  s #1 len substring$ =
    { s len #1 + global.max$ substring$ }
    's
  if$
}

%>			There are three apalike cases: one person (Jones),
%>			two (Jones and de~Bruijn), and more (Jones et~al.).
%>			This function is much like format.crossref.editors.
%>
FUNCTION {format.lab.names}
{ 's :=
  s #1 "{vv~}{ll}" format.name$
  s num.names$ duplicate$
  #2 >
    { pop$ " et~al." * }
    { #2 <
	'skip$
	{ s #2 "{ff }{vv }{ll}{ jj}" format.name$ "others" =
	    { " et~al." * }
	    { " and " * s #2 "{vv~}{ll}" format.name$ * }
	  if$
	}
      if$
    }
  if$
}

FUNCTION {author.key.label}
{ author empty$
    { key empty$
	{ cite$ #1 #3 substring$ }
	'key					%> apalike uses the whole key
      if$
    }
    { author format.lab.names }
  if$
}

FUNCTION {author.editor.key.label}
{ author empty$
    { editor empty$
	{ key empty$
	    { cite$ #1 #3 substring$ }
	    'key				%> apalike uses the whole key
	  if$
	}
	{ editor format.lab.names }
      if$
    }
    { author format.lab.names }
  if$
}

FUNCTION {editor.key.label}
{ editor empty$
    { key empty$
	{ cite$ #1 #3 substring$ }
	'key			%> apalike uses the whole key, no organization
      if$
    }
    { editor format.lab.names }
  if$
}

FUNCTION {calc.label}
{ type$ "book" =
  type$ "inbook" =
  or
    'author.editor.key.label
    { type$ "proceedings" =
	'editor.key.label			%> apalike ignores organization
	'author.key.label			%> for labeling and sorting
      if$
    }
  if$
  " ("% KCB -- replace comma w paren		%> these three lines are
  *						%> for apalike, which
  year field.or.null% purify$ #-1 #4 substring$	%> uses all four digits
  *		% KCB added % above to use entire year field as label 
  'label :=
}

FUNCTION {sort.format.names}
{ 's :=
  #1 'nameptr :=
  ""
  s num.names$ 'numnames :=
  numnames 'namesleft :=
    { namesleft #0 > }
    { nameptr #1 >
	{ "   " * }
	'skip$
      if$					%> apalike uses initials
      s nameptr "{vv{ } }{ll{ }}{  f{ }}{  jj{ }}" format.name$ 't := 
      nameptr numnames = t "others" = and
	{ "et al" * }
	{ t sortify * }
      if$
      nameptr #1 + 'nameptr :=
      namesleft #1 - 'namesleft :=
    }
  while$
}

FUNCTION {sort.format.title}
{ 't :=
  "A " #2
    "An " #3
      "The " #4 t chop.word
    chop.word
  chop.word
  sortify
  #1 global.max$ substring$
}

FUNCTION {author.sort}
{ author empty$
    { key empty$
	{ "to sort, need author or key in " cite$ * warning$
	  ""
	}
	{ key sortify }
      if$
    }
    { author sort.format.names }
  if$
}

FUNCTION {author.editor.sort}
{ author empty$
    { editor empty$
	{ key empty$
	    { "to sort, need author, editor, or key in " cite$ * warning$
	      ""
	    }
	    { key sortify }
	  if$
	}
	{ editor sort.format.names }
      if$
    }
    { author sort.format.names }
  if$
}

FUNCTION {editor.sort}
{ editor empty$
    { key empty$
	{ "to sort, need editor or key in " cite$ * warning$
	  ""
	}
	{ key sortify }
      if$
    }
    { editor sort.format.names }
  if$
}

%>			apalike uses two sorting passes; the first one sets the
%>			labels so that the `a's, `b's, etc. can be computed;
%>			the second pass puts the references in "correct" order.
%>			The presort function is for the first pass. It computes
%>			label, sort.label, and title, and then concatenates.
FUNCTION {presort}
{ calc.label
  label sortify
  "    "
  *
  type$ "book" =
  type$ "inbook" =
  or
  type$ "issue" =
  or
    'author.editor.sort
    { type$ "proceedings" =
	'editor.sort
	'author.sort
      if$
    }
  if$
  #1 entry.max$ substring$	%> for
  'sort.label :=		%> apalike
  sort.label			%> style
  *
  "    "
  *
  title field.or.null
  sort.format.title
  *
  #1 entry.max$ substring$
  'sort.key$ :=
}

ITERATE {presort}

SORT		%> by label, sort.label, title---for final label calculation

STRINGS { last.label next.extra }	%> apalike labels are only for the text;

INTEGERS { last.extra.num }		%> there are none in the bibliography

FUNCTION {initialize.extra.label.stuff}	%> and hence there is no `longest.label'
{ #0 int.to.chr$ 'last.label :=
  "" 'next.extra :=
  #0 'last.extra.num :=
}

FUNCTION {forward.pass}
{ last.label label =
    { last.extra.num #1 + 'last.extra.num :=
      last.extra.num int.to.chr$ 'extra.label :=
    }
    { "a" chr.to.int$ 'last.extra.num :=
      "" 'extra.label :=
      label 'last.label :=
    }
  if$
}

FUNCTION {reverse.pass}
{ next.extra "b" =
    { "a" 'extra.label := }
    'skip$
  if$
  label extra.label * ")" * 'label :=	%% KCB Added closing) 6-Apr-93
  extra.label 'next.extra :=
}

EXECUTE {initialize.extra.label.stuff}

ITERATE {forward.pass}

REVERSE {reverse.pass}

%>				Now that the label is right we sort for real,
%>				on sort.label then year then title.  This is
%>				for the second sorting pass.
FUNCTION {bib.sort.order}
{ sort.label
  "    "
  *
  year field.or.null sortify
  *
  "    "
  *
  title field.or.null
  sort.format.title
  *
  #1 entry.max$ substring$
  'sort.key$ :=
}

ITERATE {bib.sort.order}

SORT		%> by sort.label, year, title---giving final bibliography order

FUNCTION {begin.bib}
{ preamble$ empty$				%> no \etalchar in apalike
    'skip$
    { preamble$ write$ newline$ }
  if$
  "\begin{thebibliography}{}" write$ newline$		%> no labels in apalike
}

EXECUTE {begin.bib}

EXECUTE {init.state.consts}

ITERATE {call.type$}

FUNCTION {end.bib}
{ newline$
  "\end{thebibliography}" write$ newline$
"Done." top$
}

EXECUTE {end.bib}
