# Permit building of subdirectories only

.PHONEY: all clean cleanall subdirs $(SUBDIRS)

# The make macros below require that each subdir be enclosed in double quotes
SUBDIRS = "runtime" "compiler"

all: subdirs

subdirs: $(SUBDIRS)

cleanall: subdirs

clean: subdirs

$(SUBDIRS):
	@if [ "$@" = "$(firstword $(SUBDIRS))" ]; then echo; fi
	@$(MAKE) $(if $(filter $@,$(SUBDIRS)),-C $@) $(filter-out $(SUBDIRS) subdirs,$(MAKECMDGOALS))
	@echo


