#!/usr/bin/perl -Tw

# register.pl -- Gambit registration/download program

$data_file = '/home/arbiter/gambit_www/register.log';

use CGI;
use Fcntl;
$query = new CGI;

print <<"EndOfText";
Content-type: text/html

<HTML>
<HEAD>
<TITLE>Download Gambit</TITLE>
</HEAD>
<body text="#000000" bgcolor="#FFFFCC" link="#3333FF" vlink="#993366" alink="#993366">
<H1>Download Gambit</H1>

<p>Thank you for registering.  Click on each link to begin downloading
your selections.</p>
<HR>
<UL>
EndOfText

$name = $query->param('name');
$email = $query->param('email');
$learn = $query->param('learn');

unless ($name) {
    $name = 'Anonymous';
}
if (length($name) > 100) {
    $name = 'Someone with a really long name';
}

# encode commas
$name =~ s/,/&com;/g;

# untaint variable
unless ($name =~ /^([^<]*)$/) {
    die "couldn't untaint name: $name\n";
}
$name = $1;

unless ($email) {
    $email = 'Anonymous';
}

if (length($email) > 100) {
    $from_where = 'someone with a really long email';
}

# encode commas
$email =~ s/,/&com;/g;

# untaint variable
unless ($email =~ /^([^<]*)$/) {
    die "couldn't untaint email: $email\n";
}
$email = $1;

if (length($learn) > 1024) {
    $learn = 'a long explanation';
}
unless ($learn) {
    $learn = 'not specified';
}

# encode commas
$learn =~ s/,/&com;/g;

# untaint variable
unless ($learn =~ /^([^<]*)$/) {
    die "couldn't untaint learn: $learn\n";
}
$learn = $1;

unless ($notify = $query->param('notify'))  {
    $notify = 'no';
}

# untaint variable
unless ($notify =~ /^([^<]*)$/) {
    die "couldn't untaint notify: $notify\n";
}
$notify = $1;

if ($binmsw = $query->param('binmsw'))  {
    $binmsw = ',binmsw';
    print <<"EndOfText";
<LI><a href="ftp://ftp.hss.caltech.edu/pub/gambit/ver96/setup.exe">
Microsoft Windows 95/98/NT Executables</a>
EndOfText
}
else  {
    $binmsw = '';
}

if ($binlnx = $query->param('binlnx'))  {
    $binlnx = ',binlnx';
    print <<"EndOfText";
<LI>Linux (RedHat 6.x) Executables
EndOfText
}
else  {
    $binlnx = '';
}

if ($binsol = $query->param('binsol'))  {
    $binsol = ',binsol';
    print <<"EndOfText";
<LI>Solaris Executables
EndOfText
}
else  {
    $binsol = '';
}

if ($dochtml = $query->param('dochtml'))  {
    $dochtml = ',dochtml';
    print <<"EndOfText";
<LI>Documentation (HTML Format)
EndOfText
}
else  {
    $dochtml = '';
}

if ($src = $query->param('src'))  {
    $src = ',src';
    print <<"EndOfText";
<LI>Complete Gambit source code
EndOfText
}
else  {
    $src = '';
}

$current_time = localtime;
unless ($hostname = $query->remote_host())  {
    $hostname = 'Could not resolve host';
}

# assemble finished log entry

$entry = <<"EndOfText";
$current_time,$hostname,$name,$email,$learn,$notify$binmsw$binlnx$binsol$dochtml$src
EndOfText

# open non-destructively, read old entries, write out new

sysopen(ENTRIES, "$data_file", O_RDWR)
    or die "can't open $data_file: $!";
flock(ENTRIES, 2)        or die "can't LOCK_EX $data_file: $!";
while(<ENTRIES>) {
    $all_entries .= $_;
}
$all_entries .= $entry;

# now write out to $data_file

seek(ENTRIES, 0, 0)        or die "can't rewind $data_file: $!";
truncate(ENTRIES, 0)       or die "can't truncate $data_file: $!";
print ENTRIES $all_entries or die "can't print to $data_file: $!";
close(ENTRIES)             or die "can't close $data_file: $!";

print <<"EndOfText";
</UL>
</body>
</html>
EndOfText
