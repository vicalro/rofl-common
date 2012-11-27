#!/bin/sh

SRCDIR=$1
shift

cat <<EOF
/**
 * automatically generated by $0 $*
 * do not edit
 */

#include "matching_algorithms.h"
#include "matching_algorithms_available.h"
EOF

# write the includes
for ALG in "$@"; do
	if test -d $SRCDIR/$ALG; then
		echo "#include \"${ALG}/of12_${ALG}_match.h\""
	else
		echo "#include <rofl/pipeline/openflow/openflow12/pipeline/matching_algorithms/${ALG}/of12_${ALG}_match.h>"
	fi
done

cat <<EOF

void
load_matching_algorithm(enum matching_algorithm_available m,
		struct matching_algorithm_functions *f) 
{
	switch (m) {
EOF

# print case statements for algorithms
for ALG in "$@"; do
echo "	case matching_algorithm_$ALG:"
echo "		load_matching_algorithm_$ALG(f);"
echo "		break;"
done 

cat <<EOF
	default:
		break;
	}
}
EOF