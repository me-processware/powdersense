# PlatformIO extra script for SPIFFS upload
# This file uses SCons API which is not recognized by standard Python linters

Import("env")  # noqa: F821

# The default PlatformIO targets 'buildfs' and 'uploadfs' are already available
# No additional custom targets needed - use:
#   pio run -e <env> --target buildfs
#   pio run -e <env> --target uploadfs
