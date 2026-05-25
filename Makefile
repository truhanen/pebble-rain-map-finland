PEBBLE_EMULATOR ?= emery  # Pebble Time 2
PEBBLE_SDK ?= 4.9.169

# Needs to be run twice in a clean environment due to some initially
# missing file issues
.PHONY: build
build:
	pebble build --sdk $(PEBBLE_SDK) || pebble build --sdk $(PEBBLE_SDK)

.PHONY: install
install: build
	pebble install -vv --throttle --emulator $(PEBBLE_EMULATOR) --sdk $(PEBBLE_SDK)

.PHONY: logs
logs:
	PYTHONUNBUFFERED=1 pebble logs -vvvv --emulator $(PEBBLE_EMULATOR) --sdk $(PEBBLE_SDK)

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
