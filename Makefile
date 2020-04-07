## Format the code. Requires clang-format.
fmt:
	@echo "==> Formatting..."
	@clang-format -i src/*
	@echo "==> Formatted"

## Help

## Print a help message for using this Makefile
help:
	@$(CURDIR)/.build/scripts/help.sh $(abspath $(lastword $(MAKEFILE_LIST)))

.PHONY: fmt
