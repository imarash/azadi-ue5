# AZADI: Rise of the Dawn — build & packaging shortcuts
#
# Wraps the CI packaging scripts so you can build artifacts locally with `make`.
# See docs/BUILD.md for the full pipeline and self-hosted runner setup.
#
# Common usage:
#   make mac                       # build dist/Azadi-macOS-<version>.dmg
#   make windows                   # build Windows zip + installer (run on Windows)
#   make UE_ROOT="/path/to/UE_5.7" mac
#   make VERSION=1.2.0 mac
#   make clean                     # remove generated build/artifact folders

# --- Configurable variables -------------------------------------------------
# Engine location. Override on the command line or via the environment.
UE_ROOT ?= /Users/Shared/Epic Games/UE_5.7
# Build configuration: Development | Shipping | Test | Debug
CONFIG  ?= Shipping
# Version stamped onto artifacts. Defaults to ProjectVersion in DefaultGame.ini.
VERSION ?= $(shell grep -E '^ProjectVersion=' Config/DefaultGame.ini | head -1 | cut -d= -f2 | tr -d '[:space:]')

PROJECT := azadi.uproject
RUNUAT  := $(UE_ROOT)/Engine/Build/BatchFiles/RunUAT.sh
EDITOR_CMD := $(UE_ROOT)/Engine/Binaries/Mac/UnrealEditor-Cmd

export UE_ROOT
export AZADI_VERSION = $(VERSION)
export CONFIGURATION = $(CONFIG)

.DEFAULT_GOAL := help

# --- Targets ----------------------------------------------------------------
.PHONY: help
help: ## Show this help
	@echo "AZADI build targets (version: $(VERSION), config: $(CONFIG))"
	@echo "Engine: $(UE_ROOT)"
	@echo
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
		| sort \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-12s\033[0m %s\n", $$1, $$2}'

.PHONY: check-ue
check-ue: ## Verify UE_ROOT points at a valid engine install
	@test -x "$(RUNUAT)" || { \
		echo "ERROR: RunUAT.sh not found at '$(RUNUAT)'."; \
		echo "Set UE_ROOT, e.g.: make UE_ROOT=\"/Users/Shared/Epic Games/UE_5.7\" $(MAKECMDGOALS)"; \
		exit 1; }
	@echo "Engine OK: $(UE_ROOT)"

.PHONY: compile
compile: check-ue ## Compile the editor target (fast iteration)
	"$(UE_ROOT)/Engine/Build/BatchFiles/Mac/Build.sh" AzadiEditor Mac $(CONFIG) \
		-Project="$(CURDIR)/$(PROJECT)" -WaitMutex

.PHONY: mac
mac: check-ue ## Package macOS .dmg into dist/
	./tools/ci/package_mac.sh

.PHONY: windows
windows: ## Package Windows .zip + installer into dist/ (run on Windows w/ PowerShell)
	pwsh ./tools/ci/package_windows.ps1 -Version "$(VERSION)" -Configuration "$(CONFIG)"

.PHONY: package
package: mac ## Alias for the host platform package (macOS here)

.PHONY: clean
clean: ## Remove packaged artifacts (dist/) and staged build output (Build/)
	rm -rf dist Build

.PHONY: clean-all
clean-all: clean ## Also remove UE intermediates (Binaries, Intermediate, DDC, Saved)
	rm -rf Binaries Intermediate DerivedDataCache Saved
