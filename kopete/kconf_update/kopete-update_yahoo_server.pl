#!/usr/bin/perl

# Rename the old Gaim style to Pidgin

my $inYahoo = 0;
foreach (<>) {
    $inYahoo = 1 if (/^\[Account_YahooProtocol_.*$/);
    if ($inYahoo) {
        if (/^Server\=(.*)/) {
            my $oldServer = $1;
            if ($oldServer =~ m/\.yahoo\.com$/) {
                print "Server=scsa.msg.yahoo.com\n";
                $inYahoo = 0;
                next;
            }
        }
    }

    print $_;
}
