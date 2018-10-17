#!/bin/gawk
#
# This is auto-extract filter for specified function(s) for CSP classes.
# 2018-10-17 K.Ohta.<whatisthis.sowhat@gmail.com>
# License: GPLv2.
BEGIN {
	__N_BEGIN=0;
	__STACK=0;
}

# Set function name as regexp.
/^bool.*::process_state.*$/ {
	print $0;
	__N_BEGIN=1;
	__STACK=0;
}

# Set function name as regexp.
/^bool.*::decl_state.*$/ {
	print $0;
	__N_BEGIN=1;
	__STACK=0;
}

/^{.*$/ {
	if(__N_BEGIN != 0) 	{
		__STACK=1;#print $0;
	}
}
/^}.*$/ {
	if(__STACK != 0) print $0;
	__STACK=0;
	__N_BEGIN=0;
}

/^.*$/ {
	if(__STACK != 0) print $0;
}
