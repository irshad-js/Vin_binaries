param(
    [Parameter(Mandatory = $true)]
    [string]$SourceDir,
    [string]$OutputArchive = (Join-Path $PSScriptRoot "esp32.7z"),
    [string]$SevenZip = (Join-Path $PSScriptRoot "..\VinyaanIDE\node_modules\7zip-bin\win\x64\7za.exe")
)

$ErrorActionPreference = "Stop"
$source = (Get-Item -LiteralPath $SourceDir).FullName
if ((Split-Path -Leaf $source) -ne "esp32") {
    throw "SourceDir must point to a directory named esp32."
}
if (-not (Test-Path -LiteralPath $SevenZip -PathType Leaf)) {
    throw "7-Zip executable not found at $SevenZip"
}

$output = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputArchive)
$temporaryRoot = Join-Path ([IO.Path]::GetTempPath()) ("vinyaan-esp32-archive-" + [guid]::NewGuid())
$candidate = Join-Path $temporaryRoot "esp32.7z"
$listFile = Join-Path $temporaryRoot "files.txt"
$extractDir = Join-Path $temporaryRoot "verify"
$sourceParent = Split-Path -Parent $source

New-Item -ItemType Directory -Path $temporaryRoot | Out-Null
try {
    $entries = @(
        Get-Item -LiteralPath $source
        Get-ChildItem -LiteralPath $source -Recurse -Force
    ) | ForEach-Object {
        $_.FullName.Substring($sourceParent.Length + 1)
    }
    [Array]::Sort($entries, [StringComparer]::Ordinal)
    [IO.File]::WriteAllLines($listFile, $entries, [Text.UTF8Encoding]::new($false))

    Push-Location $sourceParent
    try {
        & $SevenZip a -t7z $candidate "@$listFile" `
            -scsUTF-8 -r- -mx=9 -m0=lzma2 -ms=on -mmt=1 `
            -mtm=off -mta=off -mtc=off -bb0
        if ($LASTEXITCODE -ne 0) {
            throw "7-Zip failed while creating the ESP32 SDK archive."
        }
    }
    finally {
        Pop-Location
    }

    New-Item -ItemType Directory -Path $extractDir | Out-Null
    & $SevenZip x $candidate "-o$extractDir" -y -bb0
    if ($LASTEXITCODE -ne 0) {
        throw "7-Zip failed while verifying the ESP32 SDK archive."
    }

    $sourceFiles = Get-ChildItem -LiteralPath $source -File -Recurse | ForEach-Object {
        [pscustomobject]@{
            Path = $_.FullName.Substring($source.Length + 1)
            Hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash
        }
    } | Sort-Object Path
    $extractedRoot = Join-Path $extractDir "esp32"
    $extractedFiles = Get-ChildItem -LiteralPath $extractedRoot -File -Recurse | ForEach-Object {
        [pscustomobject]@{
            Path = $_.FullName.Substring($extractedRoot.Length + 1)
            Hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash
        }
    } | Sort-Object Path

    if (($sourceFiles | ConvertTo-Json -Compress) -ne ($extractedFiles | ConvertTo-Json -Compress)) {
        throw "Extracted ESP32 SDK does not match the source directory."
    }

    Copy-Item -LiteralPath $candidate -Destination $output -Force
    $archiveHash = (Get-FileHash -LiteralPath $output -Algorithm SHA256).Hash.ToLowerInvariant()
    Write-Host "ESP32 SDK archive verified: $output" -ForegroundColor Green
    Write-Host "SHA256: $archiveHash"
}
finally {
    if (Test-Path -LiteralPath $temporaryRoot) {
        Remove-Item -LiteralPath $temporaryRoot -Recurse -Force
    }
}
