# Create Distribution Package Script
Write-Host "Creating Silly Shooting Game Distribution Package..." -ForegroundColor Green

$source = "c:\Users\onewi\grifting-goat\sillyShootingGame"
$destination = "$env:USERPROFILE\Downloads\SillyShootingGame_Distribution"

# Create destination folder
if (Test-Path $destination) {
    Remove-Item $destination -Recurse -Force
}
New-Item -ItemType Directory -Path $destination | Out-Null

Write-Host "Copying game files..." -ForegroundColor Yellow

# Copy essential folders and files
$itemsToCopy = @(
    "bin_win32",
    "bot", 
    "config",
    "demos",
    "docs",
    "packages", 
    "screenshots",
    "SillyShootingGame.bat",
    "GAME_README.md",
    "assaultcube.bat",
    "assaultcube_portable.bat"
)

foreach ($item in $itemsToCopy) {
    $sourcePath = Join-Path $source $item
    if (Test-Path $sourcePath) {
        $destPath = Join-Path $destination $item
        if (Test-Path $sourcePath -PathType Container) {
            Copy-Item $sourcePath $destPath -Recurse -Force
            Write-Host "  Copied folder: $item" -ForegroundColor Cyan
        } else {
            Copy-Item $sourcePath $destPath -Force  
            Write-Host "  Copied file: $item" -ForegroundColor Cyan
        }
    } else {
        Write-Host "  Warning: $item not found" -ForegroundColor Red
    }
}

Write-Host "`nCreating ZIP file..." -ForegroundColor Yellow

# Create ZIP file
$zipPath = "$env:USERPROFILE\Downloads\SillyShootingGame.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}

# Use .NET compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory($destination, $zipPath)

Write-Host "`nPackage created successfully!" -ForegroundColor Green
Write-Host "Location: $zipPath" -ForegroundColor White
Write-Host "Distribution folder: $destination" -ForegroundColor White

Write-Host "`nIMPORTANT INSTALLATION INSTRUCTIONS:" -ForegroundColor Red
Write-Host "1. Send them the ZIP file: SillyShootingGame.zip" -ForegroundColor White
Write-Host "2. They extract it anywhere on their PC" -ForegroundColor White  
Write-Host "3. FIRST TIME ONLY: Run bin_win32\oalinst.exe to install OpenAL 1.1" -ForegroundColor Yellow
Write-Host "4. Then run SillyShootingGame.bat to play!" -ForegroundColor White
Write-Host "`nNote: OpenAL 1.1 installation is required for audio to work properly." -ForegroundColor Cyan

# Optional: Open the destination folder
$openFolder = Read-Host "`nOpen destination folder? (y/n)"
if ($openFolder -eq 'y' -or $openFolder -eq 'Y') {
    Start-Process $destination
}
