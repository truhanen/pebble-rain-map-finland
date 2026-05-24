PEBBLE_EMULATOR ?= emery  # Pebble Time 2

# This may need to be run multiple times to succeed due to some initially
# missing file issues
.PHONY: build
build:
	pebble build

.PHONY: install
install: build
	pebble install -vv --emulator $(PEBBLE_EMULATOR)

.PHONY: logs
logs:
	PYTHONUNBUFFERED=1 pebble logs -vvvv --emulator ${PEBBLE_EMULATOR}

.PHONY: create_screenshots
create_screenshots:
	./scripts/create_screenshots.sh

.PHONY: clean_pebble
clean_pebble:
	pebble clean

.PHONY: clean_ts
clean_ts:
	npm run clean

.PHONY: wipe
wipe:
	pebble wipe
