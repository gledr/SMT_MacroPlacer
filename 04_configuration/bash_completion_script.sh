#! /bin/bash

#
# Howto Z-Shell:
# $ autoload bashcompinit
# $ bashcompinit
# $ source *.sh
# $ complete -F _smt_placer() -o filename smt_placer
#

_smt_placer()
{
local cur

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}

	case "$cur" in
		-*)
		COMPREPLY=( $( compgen -W ' \
			--help \
			--bash_completion \
			--save-best \
			--dump-best \
			--dump-all \
			--allsat \
			--save-all \
			--verbose \
			--store-log \
			--store-smt \
			--pareto \
			--lex \
			--box \
			--parquet \
			--partition \
			--partition-size \
			--def \
			--lef \
			--bookshelf \
			--supplement \
			--site \
			--timeout \
			--solutions \
			' -- $cur ) );;
	esac

	return 0
}

complete -F _smt_placer  smt_placer
