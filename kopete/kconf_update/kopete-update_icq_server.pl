#!/usr/bin/perl

# AOL sold ICQ; change the default server to login.icq.com

my $inICQ = 0;
foreach (<>) {
    $inICQ = 1 if (/^\[Account_ICQProtocol_.*$/);
    if ($inICQ) {
        if (/^Server\=(.*)/) {
            my $oldServer = $1;
            if ($oldServer =~ m/\.aol\.com$/) {
                print "Server=login.icq.com\n";
                $inICQ = 0;
                next;
            }
        }
    }

    print $_;
}
