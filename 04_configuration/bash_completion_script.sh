#! /bin/bash
#
# Generted File! Do not modify!
# Tue Aug 25 10:17:16 2020
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
			--store-db \
			--verbose \
			--store-log \
			--store-smt \
			--pareto \
			--lex \
			--parquet \
			--partition \
			--minimize-area \
			--minimize-hpwl \
			--free-terminals \
			--free-components \
			--skip-power-supply \
			--z3-shell \
			--z3-api \
			--partition-size \
			--partition-count \
			--def \
			--lef \
			--bookshelf \
			--supplement \
			--site \
			--timeout \
			--solutions \
			--ini \
			' -- $cur ) );;
	esac

	return 0
}

complete -F _smt_placer  smt_placer
