#Requires -Version 5.1
<#
.SYNOPSIS
  Pobiera toolchain llvm-mingw oraz deweloperskie paczki SDL3 (+ SDL3_image,
  SDL3_ttf) dla Windows do vendor/. Uzywane przez nob_win.c.

.DESCRIPTION
  Sciaga (GitHub API, najnowsze release):
    - llvm-mingw wariant ucrt-x86_64  -> vendor/llvm-mingw/
      (clang + sysroot mingw + libc++, rozpakowanie wystarcza, bez admina)
    - <prefix>-devel-<ver>-mingw.zip  -> vendor/win64/
      (naglowki + import .dll.a + .dll dla x86_64, auto-detekcja w nob_win.c)
  Ponowne uruchomienie jest bezpieczne: istniejace katalogi sa pomijane,
  chyba ze podasz -Force. VS Build Tools NIE jest potrzebne.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools/install_win_deps.ps1

.NOTES
  Wymaga dostepu do https://github.com i https://api.github.com
  (limit anonimowy API: 60 zapytan/h - skrypt robi 4).
  llvm-mingw to ~200 MB, SDL po ~1-5 MB.
#>
[CmdletBinding()]
param(
    [string]$ToolchainDir = "vendor/llvm-mingw",
    [string]$SdlDir = "vendor/win64",
    [switch]$Force
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Get-LatestRelease($repo) {
    $api = "https://api.github.com/repos/$repo/releases/latest"
    Write-Host "--- $repo"
    return Invoke-RestMethod -Uri $api -Headers @{ "User-Agent" = "sdl_gui-install-win-deps" }
}

function Get-File($url, $dst) {
    if ((Test-Path $dst) -and (-not $Force)) {
        Write-Host "OK (cache): $dst"
        return
    }
    Write-Host "Pobieranie: $url"
    Invoke-WebRequest -Uri $url -OutFile $dst
}

function Expand-ToDir($zipPath, $destDir) {
    if ((Test-Path $destDir) -and (-not $Force)) {
        Write-Host "OK (rozpakowane): $destDir"
        return
    }
    Write-Host "Rozpakowywanie do: $destDir"
    if (Test-Path $destDir) { Remove-Item -Recurse -Force $destDir }
    $tmp = "$destDir.__tmp"
    if (Test-Path $tmp) { Remove-Item -Recurse -Force $tmp }
    Expand-Archive -LiteralPath $zipPath -DestinationPath $tmp -Force
    # Zip zawiera jeden katalog nadrzedny (np. llvm-mingw-20261006-ucrt-x86_64).
    $top = Get-ChildItem $tmp | Select-Object -First 1
    New-Item -ItemType Directory -Force -Path (Split-Path $destDir) | Out-Null
    Move-Item -LiteralPath $top.FullName -Destination $destDir
    Remove-Item -Recurse -Force $tmp
}

$root = Get-Location
$tcAbs = Join-Path $root $ToolchainDir
$sdlAbs = Join-Path $root $SdlDir
$cache = Join-Path $sdlAbs "download"
New-Item -ItemType Directory -Force -Path $cache | Out-Null

# 1. llvm-mingw (wariant ucrt-x86_64)
$tcRel = Get-LatestRelease "mstorsjo/llvm-mingw"
$tcAsset = $tcRel.assets | Where-Object { $_.name -match 'ucrt-x86_64\.zip$' } | Select-Object -First 1
if (-not $tcAsset) {
    Write-Host "Dostepne assety toolchaina:"
    $tcRel.assets | ForEach-Object { Write-Host ("  " + $_.name) }
    throw "Nie znaleziono paczki llvm-mingw ucrt-x86_64 w release $($tcRel.tag_name)"
}
Write-Host "Toolchain: $($tcAsset.name)"
$tcZip = Join-Path $cache $tcAsset.name
Get-File $tcAsset.browser_download_url $tcZip
Expand-ToDir $tcZip $tcAbs

$cc = Join-Path $tcAbs "bin/x86_64-w64-mingw32-clang++.exe"
if (-not (Test-Path $cc)) { throw "Po rozpakowaniu brak $cc" }
Write-Host "Kompilator: $cc"
& $cc --version | Select-Object -First 2

# 2. SDL3 (+ image, ttf) wariant mingw
$repos = @(
    @{ Repo = "libsdl-org/SDL";       Prefix = "SDL3" },
    @{ Repo = "libsdl-org/SDL_image"; Prefix = "SDL3_image" },
    @{ Repo = "libsdl-org/SDL_ttf";   Prefix = "SDL3_ttf" }
)

foreach ($r in $repos) {
    $rel = Get-LatestRelease $r.Repo
    $tag = $rel.tag_name            # np. "release-3.4.18"
    $ver = $tag -replace '^release-', ''
    $asset = "$($r.Prefix)-devel-$ver-mingw.zip"
    $url = "https://github.com/$($r.Repo)/releases/download/$tag/$asset"
    Write-Host "Biblioteka: $($r.Prefix) $ver"
    $zip = Join-Path $cache $asset
    Get-File $url $zip
    if (Test-Path $sdlAbs) {
        # Rozpakuj wprost do SdlDir (zip ma wlasny katalog nadrzedny SDL3-3.4.18/).
        $probe = Join-Path $sdlAbs "$($r.Prefix)-$ver"
        if ((Test-Path $probe) -and (-not $Force)) {
            Write-Host "OK (rozpakowane): $probe"
            continue
        }
        Write-Host "Rozpakowywanie do: $sdlAbs"
        Expand-Archive -LiteralPath $zip -DestinationPath $sdlAbs -Force
    } else {
        Expand-ToDir $zip $sdlAbs   # nie powinno sie zdarzyc (cache juz utworzyl)
    }
}

Write-Host ""
Write-Host "Zawartosc ${sdlAbs}:"
Get-ChildItem $sdlAbs | Format-Table Name, Mode
Write-Host ""
Write-Host "Dalej (kompilacja narzedzia buildujacego wlasnym toolchainem):"
Write-Host "  .\vendor\llvm-mingw\bin\x86_64-w64-mingw32-clang.exe -o nob_win.exe nob_win.c"
Write-Host "  .\nob_win.exe test"
Write-Host ""
Write-Host "Opcjonalne (przywracaja skipowane przyklady):"
Write-Host "  - glslc z Vulkan SDK (https://vulkan.lunarg.com) -> przyklady 40/41"
Write-Host "  - SDL3_mixer-devel-*-mingw.zip do vendor/win64/  -> przyklady *mixer*"
Write-Host "  - sqlite3 (amalgamacja .h + .c wprost do zrodla lub .a do vendor) -> przyklady *sqlite*"
