#!/usr/bin/perl -w
# Author: zack@kde.org
# This script is licensed under :
# Sausage license - you are permited to copy, steal and use
# the code contained in this file. You're not allowed to inflict
# serious physical damage as a direct result of this script. You
# agree to clean my apartment at least once a week and do 50 pushups
# in the unlikely event of not fullfilling the above mentioned conditions.

# This two will be inserted at the end of all files
our $emacs_format ="/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */";
our $vim_format = "// vim: set noet ts=4 sts=4 sw=4:";


# This two will be deleted from all files!
# change them accordingly if you wish to switch styles

our $old_emacs_f = "/\\*\n \\* Local variables:\n \\* c-indentation-style: k&r\n \\* c-basic-offset: 8\n \\* indent-tabs-mode: t\n \\* End:\n \\*/";
our $vim1_f = "// vim: set noet ts=4 sts=4 sw=4:";
our $vim2_f = "// vim: set ts=4 sts=4 sw=4 noet:";
our $vim3_f = "// vim: set noet sw=4 ts=4 sts=4:";
our $vim4_f = "// vim: set noet ts=4 sw=4 sts=4:";
our $old_vim_f = "$vim1_f|$vim2_f|$vim3_f|$vim4_f";

##### Code ### don't change anything after this ####

die "Wrong number of arguments!\n" if ( @ARGV != 1);

print "Looking for files...\n";
my @files = `find $ARGV[0] -name \"*.h\" -or -name \"*.cpp\" -or -name \"*.cc\"`;

foreach my $file (@files) {
    open( SRC, "< $file" ) or die "Couldn't open file for reading $ARGV[0]\n";
    my $contents = join "", <SRC>;
    close SRC;

    $contents =~ s/$old_vim_f//g;
    $contents =~ s/$old_emacs_f//g;
    $contents =~ s/$/\n$emacs_format\n$vim_format\n/;

    open( SRC, "> $file" ) or die "Couldn't open file for writing $ARGV[0]\n";

    print SRC $contents;
    close SRC;
}
