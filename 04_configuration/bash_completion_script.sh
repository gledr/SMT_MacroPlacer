#! /bin/bash
#
# Generted File! Do not modify!
# Fri Jul 17 10:13:59 2020
#

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
			--bash-completion \
			--save-best \
			--dump-best \
			--dump-all \
			--save-all \
			--verbose \
			--store-log \
			--store-smt \
			--pareto \
			--lex \
			--minizinc \
			--parquet \
			--partition \
			--minimize-area \
			--minimize-hpwl \
			--free-terminals \
			--skip-power-supply \
			--partition-size \
			--partition-count \
			--def \
			--lef \
			--bookshelf \
			--supplement \
			--site \
			--timeout \
			--Z3_API \
			--solutions \
			--ini \
			' -- $cur ) );;
	esac

	return 0
}

complete -F _smt_placer  smt_placer
