#!/bin/bash
grep "#include" *.cpp *.h | sed -e 's/^.*\://' | sort | uniq > pch_list
