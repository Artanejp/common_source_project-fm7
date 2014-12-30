BEGIN {
	    printf "const char* %s = \"", VARNAME;
}

/^.*/ {
        gsub("\"", "\\\"", $0);
	printf "%s\\n", $0;
}

END {
    printf "\";\n"
}