#!/bin/bash
# Skrypt do pobierania deweloperskich bibliotek SDL2, SDL2_image i SDL2_ttf
# dla cross-kompilacji pod Windows (MinGW-w64).

set -e # Zakończ w przypadku błędu

# Wersje bibliotek i nazwy plików
SDL_VERSION="2.30.5"
SDL_IMAGE_VERSION="2.8.2"
SDL_TTF_VERSION="2.22.0"

SDL_FILE="SDL2-devel-${SDL_VERSION}-mingw.zip"
SDL_IMAGE_FILE="SDL2_image-devel-${SDL_IMAGE_VERSION}-mingw.zip"
SDL_TTF_FILE="SDL2_ttf-devel-${SDL_TTF_VERSION}-mingw.zip"

SDL_URL="https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/${SDL_FILE}"
SDL_IMAGE_URL="https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL_IMAGE_VERSION}/${SDL_IMAGE_FILE}"
SDL_TTF_URL="https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL_TTF_VERSION}/${SDL_TTF_FILE}"

# Katalog docelowy na zależności
DEPS_DIR="vendor/win64_deps"
DOWNLOAD_DIR="${DEPS_DIR}/download"

echo "--- Przygotowanie katalogów ---"
mkdir -p "${DOWNLOAD_DIR}"
echo "Katalog docelowy: ${DEPS_DIR}"

# Funkcja do pobierania i rozpakowywania
download_and_unzip() {
    local url=$1
    local file=$2
    
    echo "Pobieranie ${file}..."
    curl -L -o "${DOWNLOAD_DIR}/${file}" "${url}"
    
    echo "Rozpakowywanie ${file}..."
    unzip -q -o "${DOWNLOAD_DIR}/${file}" -d "${DEPS_DIR}"

    # Zmieniamy nazwę rozpakowanego folderu na bardziej przewidywalną
    # Np. SDL2-2.30.5 -> SDL2
    local extracted_dir_name=$(unzip -Z1 "${DOWNLOAD_DIR}/${file}" | head -n 1 | cut -d'/' -f1)
    local target_dir_name=$(echo "${extracted_dir_name}" | cut -d'-' -f1)
    
    if [ -d "${DEPS_DIR}/${extracted_dir_name}" ]; then
        # Usuń stary katalog docelowy, jeśli istnieje, aby uniknąć błędów
        rm -rf "${DEPS_DIR}/${target_dir_name}"
        mv "${DEPS_DIR}/${extracted_dir_name}" "${DEPS_DIR}/${target_dir_name}"
        echo "Zmieniono nazwę ${extracted_dir_name} na ${target_dir_name}"
    fi
}

# Pobieranie i rozpakowywanie wszystkich bibliotek
download_and_unzip "${SDL_URL}" "${SDL_FILE}"
download_and_unzip "${SDL_IMAGE_URL}" "${SDL_IMAGE_FILE}"
download_and_unzip "${SDL_TTF_URL}" "${SDL_TTF_FILE}"

echo "--- Czyszczenie pobranych archiwów ---"
rm -rf "${DOWNLOAD_DIR}"

echo ""
echo "Pobieranie i rozpakowywanie zależności zakończone pomyślnie."
echo "Biblioteki znajdują się w katalogu: ${DEPS_DIR}"