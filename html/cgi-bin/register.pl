#!/software/bin/perl -Tw

# register.pl -- Gambit registration/download program

$data_file = '/home/gambit/public_html/logs/register.log';

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
<p>When installing executables, we highly recommend that you
refer to the <tt>README.1ST</tt> file for
platform-specific instructions and hints.</p>
<HR>
<UL>
EndOfText

$name = $query->param('name');
$email = $query->param('email');
$learn = $query->param('learn');
unless ($site = $query->param('site'))  {
    $site = 'US';
}

if ($site eq 'CH')  {
    $basepath = 'http://www.inf.ethz.ch/personal/stengel/gambit/ver96.3';
}
else  {
    $basepath = 'ftp://ftp.hss.caltech.edu/pub/gambit/ver96.3';
}

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
    $email = 'someone with a really long email';
}

# encode commas
$email =~ s/,/&com;/g;

# untaint variable
unless ($email =~ /^([^<]*)$/) {
    die "couldn't untaint email: $email\n";
}
$email = $1;

unless ($learn) {
    $learn = 'not specified';
}
if (length($learn) > 1024) {
    $learn = 'a long explanation';
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
<LI><a href="$basepath/setup.exe">
Microsoft Windows 95/98/NT Executables</a>
EndOfText
}
else  {
    $binmsw = '';
}

if ($binlnx = $query->param('binlnx'))  {
    $binlnx = ',binlnx';
    print <<"EndOfText";
<LI><a href="$basepath/gambit_motif_Linux.zip">Linux (RedHat 6.0) Executables</a>
EndOfText
}
else  {
    $binlnx = '';
}

if ($binsol = $query->param('binsol'))  {
    $binsol = ',binsol';
    print <<"EndOfText";
<LI><a href="$basepath/gambit_motif_SunOS.zip">Solaris Executables</a>
EndOfText
}
else  {
    $binsol = '';
}

if ($guidochtml = $query->param('guidochtml'))  {
    $guidochtml = ',guidochtml';
    print <<"EndOfText";
<LI><a href="$basepath/guiman.html.zip">GUI Documentation (HTML Format)</a>
EndOfText
}
else  {
    $guidochtml = '';
}

if ($guidocpdf = $query->param('guidocpdf'))  {
    $guidocpdf = ',guidocpdf';
    print <<"EndOfText";
<LI><a href="$basepath/guiman.pdf">GUI Documentation (PDF Format)</a>
EndOfText
}
else  {
    $guidocpdf = '';
}

if ($guidocps = $query->param('guidocps'))  {
    $guidocps = ',guidocps';
    print <<"EndOfText";
<LI><a href="$basepath/guiman.ps">GUI Documentation (PostScript Format)</a>
EndOfText
}
else  {
    $guidocps = '';
}

if ($gcldocps = $query->param('gcldocps'))  {
    $gcldocps = ',gcldocps';
    print <<"EndOfText";
<LI><a href="$basepath/gclman.ps">GCL Documentation (PostScript Format)</a>
EndOfText
}
else  {
    $gcldocps = '';
}

if ($srcunx = $query->param('srcunx'))  {
    $srcunx = ',srcunx';
    print <<"EndOfText";
<LI><a href="$basepath/gunixsrc.zip">Complete Gambit source code (Unix)</a>
EndOfText
}
else  {
    $srcunx = '';
}

if ($srcmsw = $query->param('srcmsw'))  {
    $srcmsw = ',srcmsw';
    print <<"EndOfText";
<LI><a href="$basepath/gwinsrc.zip">Complete Gambit source code (Microsoft Windows)</a>
EndOfText
}
else  {
    $srcmsw = '';
}

if ($wxmsw = $query->param('wxmsw'))  {
    $wxmsw = ',wxmsw';
    print <<"EndOfText";
<LI><a href="$basepath/wx168.zip">wxWindows 1.68 libraries for Microsoft Windows</a>
EndOfText
}
else {
    $wxmsw = '';
}

if ($wxlnx = $query->param('wxlnx'))  {
    $wxlnx = ',wxlnx';
    print <<"EndOfText";
<LI><a href="$basepath/wx_motif_Linux.zip">wxWindows 1.68 libraries for RedHat Linux 6.0</a>
EndOfText
}
else {
    $wxlnx = '';
}

if ($wxsol = $query->param('wxsol'))  {
    $wxsol = ',wxsol';
    print <<"EndOfText";
<LI><a href="$basepath/wx_motif_SunOS.zip">wxWindows 1.68 libraries for Solaris</a>
EndOfText
}
else {
    $wxsol = '';
}

$current_time = localtime;
unless ($hostname = $query->remote_host())  {
    $hostname = 'Could not resolve host';
}

# assemble finished log entry

$entry = <<"EndOfText";
$current_time,$hostname,$name,$email,$learn,$site,$notify$binmsw$binlnx$binsol$guidochtml$guidocpdf$guidocps$gcldocps$srcmsw$srcunx$wxmsw$wxlnx$wxsol
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
