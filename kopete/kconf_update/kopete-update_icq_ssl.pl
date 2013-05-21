#!/usr/bin/perl

# ICQ protocol in Kopete now support SSL encryption
# Enable it by default for security reasons

my @settings;
my $inICQ = 0;
my $indexServer = -1;
my $indexPort = -1;
my $index = 0;

foreach (<>) {
    push @settings, $_;
    if (/^\[Account_.*]$/) {
        $indexServer = -1;
        $indexPort = -1;
        if (/^\[Account_ICQProtocol_.*$/) {
            $inICQ = 1;
        } else {
            $inICQ = 0;
        }
    }
    if ($inICQ) {
        if (/^Server\=(.*)/) {
            my $oldServer = $1;
            if ($oldServer =~ m/^(login\.icq|.*\.aol)\.com\s*$/) {
                $indexServer = $index;
            }
        } elsif (/^Port\=(.*)/) {
            my $oldPort = $1;
            if ($oldPort =~ m/^5190\s*$/ ) {
                $indexPort = $index;
            }
        }
        if ( $indexServer >= 0 and $indexPort >= 0 ) {
            push @settings, "Encrypted=true\n";
            $settings[$indexServer] = "Server=slogin.icq.com\n";
            $settings[$indexPort] = "Port=443\n";
            $inICQ = 0;
            $indexServer = -1;
            $indexPort = -1;
        }
    }
    $index++;
}

foreach (@settings) {
    print $_;
}
