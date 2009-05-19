#!/usr/bin/perl

# Rename the old Gaim style to Pidgin

foreach (<>) {
    if(/^styleName=Gaim\n$/) {
        print "styleName=Pidgin\n";
        next;
    }
    print $_;
}
