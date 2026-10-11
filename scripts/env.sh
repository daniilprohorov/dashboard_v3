# Shared toolchain pin. Sourced by other scripts.
IDF_VERSION="v5.5.5"
IDF_PATH="${IDF_PATH_OVERRIDE:-$HOME/esp/esp-idf-$IDF_VERSION}"
export IDF_PATH
# IDF venv is built for Python 3.12; export.sh picks the first python3 on PATH.
if [ -d /opt/homebrew/opt/python@3.12/libexec/bin ]; then
  export PATH="/opt/homebrew/opt/python@3.12/libexec/bin:$PATH"
fi
